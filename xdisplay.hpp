#ifndef ALS_XEUS_CLING_XDISPLAY_HPP
#define ALS_XEUS_CLING_XDISPLAY_HPP

#include "nlohmann/json.hpp"
namespace nl = nlohmann;

#include "xinterpreter.hpp"
#include <als-basic-utilities/ToString.hpp>

extern als::xeus_cling::interpreter* xci;

namespace als::xeus_cling
{
    // Mime representation latex and plain.
    template<class T, class... Args>
    nl::json inline mime_representation_plain(const T& object, Args... args)
    {
        nl::json res;
        res["text/plain"] = als::utilities::to_plain(object, args...);
        return res;
    }

    template<class T, class... Args>
    nl::json inline mime_representation_latex(const T& object, Args... args)
    {
        nl::json res;
        res["text/latex"] = "$" + als::utilities::to_latex(object, args...) + "$";
        return res;
    }

    template<class T, class... Args>
    nl::json inline mime_representation(const T& object,
        const als::utilities::RepresentationType rt, Args... args)
    {
        return (rt == als::utilities::RepresentationType::LATEX) ? 
            mime_representation_latex(object, args...) :
            mime_representation_plain(object, args...);
    }

    // Display functions.
    template<class T, class... Args>
    void inline display(const T& object,
        const als::utilities::RepresentationType rt, Args... args)
    {
        xci->display_data(mime_representation(object, rt, args...),
            nl::json::object(), nl::json::object());
    }

    template<class T, class... Args>
    void inline display_plain(const T& object, Args... args)
    {
        xci->display_data(mime_representation_plain(object, args...),
            nl::json::object(), nl::json::object());
    }

    template<class T, class... Args>
    void inline display_latex(const T& object, Args... args)
    {
        xci->display_data(mime_representation_latex(object, args...),
            nl::json::object(), nl::json::object());
    }
}

#endif // ALS_XEUS_CLING_XDISPLAY_HPP