/***********************************************************************************
* Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht *
* Copyright (c) 2016, QuantStack                                                   *
*                                                                                  *
* Distributed under the terms of the BSD 3-Clause License.                         *
*                                                                                  *
* The full license is in the file LICENSE, distributed with this software.         *
************************************************************************************/

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "xparser.hpp"

namespace xcpp
{
    std::vector<std::string> split_line(const std::string& input, const std::string& delims, std::size_t cursor_pos)
    {
        // passing -1 as the submatch index parameter performs splitting
        std::vector<std::string> result;
        std::stringstream ss;

        ss << "[";
        for (auto c : delims)
        {
            ss << "\\" << c;
        }
        ss << "]";

        std::regex re(ss.str());

        std::copy(std::sregex_token_iterator(input.begin(), input.begin() + cursor_pos + 1, re, -1),
                  std::sregex_token_iterator(),
                  std::back_inserter(result));

        return result;
    }
}
