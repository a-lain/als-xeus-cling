#ifndef ALS_XEUS_CLING_INTERPRETER_HPP
#define ALS_XEUS_CLING_INTERPRETER_HPP

#include <string>
#include "nlohmann/json.hpp"
#include "als-xeus-cling-config.hpp"
#include "xeus/xinterpreter.hpp"
#include <cling/Interpreter/Interpreter.h>
#include <cling/MetaProcessor/InputValidator.h>
#include <als-basic-utilities/ToString.hpp>


namespace nl = nlohmann;

namespace als::xeus_cling
{
    /**
     * @brief The xeus-interpreter.
     * 
     */
    class ALS_XEUS_CLING_API interpreter : public xeus::xinterpreter
    {
        public:

        interpreter();
        virtual ~interpreter() = default;

        /**
         * @brief Allows to perform some operations after the custom_interpreter creation
         * and before executing any request. 
         * 
         */
        void configure_impl() override;

        /**
         * @brief Code execution request from the client.
         * 
         * @param execution_counter Typically the cell number.
         * @param code Source code to be executed by the kernel, one or more lines.
         * @param silent A boolean flag which, if true,
         * signals the kernel to execute this code as quietly as possible.
         * silent=True forces store_history to be false, and will *not*
         * broadcast output on the IOPUB channel or have an execute_result.
         * The default is false.
         * @param store_history A boolean flag which, if true, signals the kernel
         * to populate history. The default is true if silent is false. If silent 
         * is true, store_history is forced to be false.
         * @param user_expressions A dict mapping names to expressions to be evaluated
         * in the user's dict. The rich display-data representation of each will be
         * evaluated after execution. See the display_data content for the structure 
         * of the representation data.
         * @param allow_stdin Some frontends do not support stdin requests. If this
         * is true, code running in the kernel can prompt the user for input with an
         * input_request message (see below). If it is false, the kernel should not
         * send these messages.
         * @return nl::json 
         */
        nl::json execute_request_impl(int execution_counter, const std::string& code,
            bool silent, bool store_history, nl::json user_expressions,
            bool allow_stdin) override;

        /**
         * @brief Code completion request from the client.
         * 
         * @param code The code context in which completion is requested. This may be up
         * to an entire multiline cell.
         * @param cursor_pos The cursor position within 'code' (in unicode characters) 
         * where completion is requested.
         * @return nl::json 
         */
        nl::json complete_request_impl(const std::string& code, int cursor_pos) override;

        /**
         * @brief Code inspection request (using a question mark on a type for example).
         * 
         * @param code The code context in which introspection is requested. This may be up
         * to an entire multiline cell.
         * @param cursor_pos The cursor position within 'code' (in unicode characters) where
         * inspection is requested.
         * @param detail_level The level of detail desired.  In IPython, the default (0) is
         * equivalent to typing 'x?' at the prompt, 1 is equivalent to 'x??'. The difference is
         * up to kernels, but in IPython level 1 includes the source code if available.
         * @return nl::json 
         */
        nl::json inspect_request_impl(const std::string& code,
                                      int cursor_pos,
                                      int detail_level) override;

        /**
         * @brief Called before code execution (terminal mode) in order to check if the code is
         * complete and can be executed as it is.
         * 
         * For example, when typing a for loop on multiple lines in Python, code will be considered
         * complete when the for loop has been closed).
         * 
         * @param code The code entered so far as a multiline string.
         * @return nl::json 
         */
        nl::json is_complete_request_impl(const std::string& code) override;

        /**
         * @brief nformation request about the kernel: language name (for code highlighting), 
         * language version, terminal banner etc.
         * 
         * @return nl::json 
         */
        nl::json kernel_info_request_impl() override;

        /**
         * @brief Shutdown request from the client, this allows you to do some extra work before
         * the kernel is shut down (e.g. free allocated memory).
         * 
         */
        void shutdown_request_impl() override;

        cling::Interpreter cling_interpreter;
        cling::InputValidator cling_input_validator;
        als::utilities::RepresentationType display_preferencies;
    };
}

#endif
