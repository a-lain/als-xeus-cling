/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#ifndef XCPP_PARSER_HPP
#define XCPP_PARSER_HPP

#include <map>
#include <string>
#include <vector>

namespace xcpp
{
    std::vector<std::string> split_line(const std::string& input,
        const std::string& delims, std::size_t cursor_pos);
}

#endif // XCPP_PARSER_HPP