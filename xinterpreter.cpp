#include <string>
#include <vector>
#include <iostream>

#include "nlohmann/json.hpp"

#include "xeus/xinput.hpp"
#include "xeus/xhelper.hpp"

#include "xinterpreter.hpp"
#include "xparser.hpp"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <cling/Interpreter/Exception.h>
#include <cling/MetaProcessor/InputValidator.h>
#include <clang/AST/Type.h>

#include <filesystem>
#include <regex>

namespace nl = nlohmann;

namespace als::xeus_cling
{
    interpreter::interpreter(): cling_interpreter{cling::Interpreter(2,
            std::vector<char*>({"xeus-cling", "-std=c++17"}).data())},
            cling_input_validator({cling::InputValidator()}),
            display_preferencies{als::utilities::RepresentationType::PLAIN}
    {
        // We add necessary includes.
        cling_interpreter.AddIncludePath(ALS_CLANG_INCLUDE_PATH);
        cling_interpreter.AddIncludePath(ALS_CLING_INCLUDE_PATH);
        cling_interpreter.AddIncludePath(std::filesystem::current_path().string());

        // We expose the interpreter to cling as a pointer called xci.
        cling_interpreter.process("#include <als-xeus-cling/xinterpreter.hpp>", nullptr, nullptr, false);
        cling_interpreter.process(
            "als::xeus_cling::interpreter* xci = (als::xeus_cling::interpreter*)" +
                std::to_string(intptr_t(this)) + ";",
            nullptr, nullptr, false);

        // We include display.hpp.
        cling_interpreter.process("#include <als-xeus-cling/xdisplay.hpp>", nullptr, nullptr, false);

        // We register the interpreter.
        xeus::register_interpreter(this);
    }

    nl::json interpreter::execute_request_impl(int execution_counter,
        const std::string& code, bool silent, bool store_history,
        nl::json user_expressions, bool allow_stdin)
    {
        // 1. We create the return value.
        nl::json kernel_res;

        // 2. We prepare cling objects for the cling interpreter.
        cling::Value output;
        cling::Interpreter::CompilationResult compilation_result =
            cling::Interpreter::kFailure;

        // 3. We redirect std::cout and std::cerr outputs.
        std::stringstream output_buffer;
        std::stringstream error_buffer;
        // This next line saves old cerr buffer into old_error and writes the new one
        // simultanously.
        std::streambuf* old_output = std::cout.rdbuf(output_buffer.rdbuf());
        std::streambuf* old_error = std::cerr.rdbuf(error_buffer.rdbuf());

        // 4. We process the cell code via cling interpreter. This part is almost
        // copied from xeus-cling implementation.
        bool error_has_ocurred = false;
        std::string error_name;
        try
        {
            // If last line of code does not start with "#", we add a semicolon at the end.
            if (code[code.find_last_of("\n") + 1] != '#')
            {
                compilation_result = cling_interpreter.process(code + ";", &output);
            }
            else
            {
                compilation_result = cling_interpreter.process(code, &output);
            }
            
        }
        catch(const cling::InterpreterException& e)
        {
            error_has_ocurred = true;
            error_name = "Interpreter Exception";
            if (!e.diagnose())
            {
                std::cerr << e.what();
            }
        }
        catch (const std::exception& e)
        {
            error_has_ocurred = true;
            error_name = "Standard Exception";
            std::cerr << e.what();
        }
        catch (...)
        {
            error_has_ocurred = true;
            error_name = "Unkown error";
        }

        if (compilation_result != cling::Interpreter::kSuccess)
        {
            error_has_ocurred = true;
            error_name = "Interpreter error";
        }

        // 5. We revert std::cout and std::cerr outputs.
        std::cout.rdbuf(old_output);
        std::cerr.rdbuf(old_error);

        // 6. We publish the result or the error.
        if (error_has_ocurred)
        {
            std::vector<std::string> traceback({error_name + ": " + error_buffer.str()});
            publish_execution_error(error_name, error_buffer.str(), traceback);

            kernel_res["status"] = "error";
            kernel_res["ename"] = error_name;
            kernel_res["evalue"] = error_buffer.str();
            kernel_res["traceback"] = traceback;
        }
        else
        {
            // 6.a. We publish all captured std:cerr output.
            if (!error_buffer.str().empty())
            {
                nl::json warnings;
                warnings["text/plain"] = "Execution of the cell has captured the following errors:\n"
                    + error_buffer.str();
                display_data(warnings, nl::json::object(), nl::json::object());
            }

            // 6.b. We publish all captured std::cout output.
            if (!output_buffer.str().empty())
            {
                nl::json text_output;
                text_output["text/plain"] = "Execution of the cell has captured the following outputs:\n"
                    + output_buffer.str();
                display_data(text_output, nl::json::object(), nl::json::object());
            }

            // 6.c. We display the last object as output if a semicolon was omitted
            // in the last line.
            if (output.hasValue() && code[code.find_last_not_of(' ')] != ';')
            {
                // We define the object that is going to be displayed as output.
                nl::json result;
                
                // We obtain a string representing the output type (with no reference types),
                // i.e., we will never obtain int&, we will obtain int. 
                std::string output_type = output.getType().getNonReferenceType().getAsString();
                
                // This pointer will point to the adress where our output is stored.
                void* output_adress = nullptr;
                // Because of how cling works, objects are stored differently depending on their
                // type. If our object is of one of the types listed below, the cling::Value object
                // contains its actual value and not a pointer to it. In all other cases,
                // cling::Value object stores a pointer to our object.
                if (output_type == "float")
                {
                    output_adress = &output.getFloat();
                }
                else if (output_type == "double")
                {
                    output_adress = &output.getDouble();
                }
                else if (output_type == "long double")
                {
                    output_adress = &output.getLongDouble();
                }
                else if (output_type == "char" ||
                         output_type == "int" ||
                         output_type == "long int" ||
                         output_type == "long long int")
                {
                    output_adress = &output.getLL();
                }
                else if (output_type == "unsigned char" ||
                         output_type == "unsigned int" ||
                         output_type == "unsigned long int" ||
                         output_type == "unsigend long long int")
                {
                    output_adress = &output.getULL();
                }
                else
                {
                    output_adress = output.getPtr();
                }

                // This cling::Value is going to store the nl::json object returned by
                // mime_representation.
                cling::Value output_mime_representation;
                // This is the code that is going to be compiled to obtain the representation.
                std::stringstream representation_code;
                representation_code << "als::xeus_cling::mime_representation(*("
                    << output_type << "*)"
                    << output_adress << ", xci->display_preferencies);;";

                // Again, we redirect std::cout and std::cerr outputs.
                output_buffer.clear();
                error_buffer.clear();
                old_output = std::cout.rdbuf(output_buffer.rdbuf());
                old_error = std::cerr.rdbuf(error_buffer.rdbuf());

                // We try to compile the representation code.
                try
                {
                    cling_interpreter.process(representation_code.str(), &output_mime_representation,
                        nullptr, false);
                }
                catch(const cling::InterpreterException& e)
                {
                    error_has_ocurred = true;
                    error_name = "Interpreter Exception while evaluating output";
                    if (!e.diagnose())
                    {
                        std::cerr << e.what();
                    }
                }
                catch (const std::exception& e)
                {
                    error_has_ocurred = true;
                    error_name = "Standard Exception while evaluating output";
                    std::cerr << e.what();
                }
                catch (...)
                {
                    error_has_ocurred = true;
                    error_name = "Unkown error while evaluating output";
                }

                if (compilation_result != cling::Interpreter::kSuccess)
                {
                    error_has_ocurred = true;
                    error_name = "Interpreter error while evaluating output";
                }
                
                // We revert std::cout and std::cerr outputs.
                std::cout.rdbuf(old_output);
                std::cerr.rdbuf(old_error);

                if (error_has_ocurred)
                {
                    std::vector<std::string> traceback({error_name + ": " + error_buffer.str()});
                    publish_execution_error(error_name, error_buffer.str(), traceback);

                    kernel_res["status"] = "error";
                    kernel_res["ename"] = error_name;
                    kernel_res["evalue"] = error_buffer.str();
                    kernel_res["traceback"] = traceback;
                }
                else
                {
                    // We publish all captured std:cerr output.
                    if (!error_buffer.str().empty())
                    {
                        nl::json warnings;
                        warnings["text/plain"] = "Evaluation of the output has captured the following errors:\n"
                            + error_buffer.str();
                        display_data(warnings, nl::json::object(), nl::json::object());
                    }

                    // We publish all captured std::cout output.
                    if (!output_buffer.str().empty())
                    {
                        nl::json text_output;
                        text_output["text/plain"] = "Evaluation of the output has captured the following outputs:\n"
                            + output_buffer.str();
                        display_data(text_output, nl::json::object(), nl::json::object());
                    }

                    // Finally, we publish the evaluation of the output.
                    publish_execution_result(execution_counter,
                        *(nl::json*)output_mime_representation.getPtr(), nl::json::object());
                }
            }

            kernel_res["status"] = "ok";
            kernel_res["payload"] = nl::json::array();
            kernel_res["user_expressions"] = nl::json::object();
        }

        return kernel_res;
       
    }

    void interpreter::configure_impl()
    {
        // Perform some operations
    }

    nl::json interpreter::is_complete_request_impl(const std::string& code)
    {
        // Copied from xeus-cling implementation.
        nl::json kernel_res;

        cling_input_validator.reset();
        cling::InputValidator::ValidationResult res = cling_input_validator.validate(code);
        if (res == cling::InputValidator::kComplete)
        {
            kernel_res["status"] = "complete";
        }
        else if (res == cling::InputValidator::kIncomplete)
        {
            kernel_res["status"] = "incomplete";
        }
        else if (res == cling::InputValidator::kMismatch)
        {
            kernel_res["status"] = "invalid";
        }
        else
        {
            kernel_res["status"] = "unknown";
        }
        kernel_res["indent"] = "";
        return kernel_res;
    }

    nl::json interpreter::complete_request_impl(const std::string&  code,
                                                     int cursor_pos)
    {
        // Copied from xeus-cling implementation.
        std::vector<std::string> result;
        cling::Interpreter::CompilationResult compilation_result;
        nl::json kernel_res;

        // split the input to have only the word in the back of the cursor
        std::string delims = " \t\n`!@#$^&*()=+[{]}\\|;:\'\",<>?.";
        std::size_t _cursor_pos = cursor_pos;
        std::vector<std::string> text = xcpp::split_line(code, delims, _cursor_pos);
        std::string to_complete = text.back().c_str();

        compilation_result = cling_interpreter.codeComplete(code.c_str(), _cursor_pos, result);

        // change the print result
        for (std::string& r : result)
        {
            // remove the definition at the beginning (for example [#int#])
            r = std::regex_replace(r, std::regex("\\[\\#.*\\#\\]"), "");
            // remove the variable name in <#type name#>
            r = std::regex_replace(r, std::regex("(\\ |\\*)+(\\w+)(\\#\\>)"), "$1$3");
            // remove unnecessary space at the end of <#type   #>
            r = std::regex_replace(r, std::regex("\\ *(\\#\\>)"), "$1");
            // remove <# #> to keep only the type
            r = std::regex_replace(r, std::regex("\\<\\#([^#>]*)\\#\\>"), "$1");
        }

        kernel_res["matches"] = result;
        kernel_res["cursor_start"] = cursor_pos - to_complete.length();
        kernel_res["cursor_end"] = cursor_pos;
        kernel_res["metadata"] = nl::json::object();
        kernel_res["status"] = "ok";
        return kernel_res;
    }

    nl::json interpreter::inspect_request_impl(const std::string& code,
        int cursor_pos, int detail_level)
    {
        // TODO. Seems complicated.

        nl::json kernel_res;
        kernel_res["plain/text"] = "Sorry. Code inspection is not yet implemented.";
        return kernel_res;
    }

    void interpreter::shutdown_request_impl()
    {
        std::cout << "Bye!!" << std::endl;
    }

    nl::json interpreter::kernel_info_request_impl()
    {

        const std::string protocol_version = "5.3";
        const std::string implementation = "als-xeus-cling";
        const std::string implementation_version = ALS_XEUS_CLING_VERSION;
        const std::string language_name = "c++";
        const std::string language_version = "c++17";
        const std::string language_mimetype = "text/x-c++src";
        const std::string language_file_extension = ".cpp";
        const std::string language_pygments_lexer = "";
        const std::string language_codemirror_mode = "text/x-c++src";
        const std::string language_nbconvert_exporter = "";
        const std::string banner = "als-xeus-cling-kernel";
        const bool debugger = false;
        const nl::json help_links = nl::json::array();


        return xeus::create_info_reply(
            protocol_version,
            implementation,
            implementation_version,
            language_name,
            language_version,
            language_mimetype,
            language_file_extension,
            language_pygments_lexer,
            language_codemirror_mode,
            language_nbconvert_exporter,
            banner,
            debugger,
            help_links
        );
    }

}
