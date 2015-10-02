#pragma once

#include <cctype>
#include <string>
#include <algorithm>

namespace text
{
        ///
        /// \brief check if a string starts with a token (case sensitive)
        ///
        inline bool starts_with(const std::string& str, const std::string& token)
        {
                return  str.size() >= token.size() &&
                        std::equal(token.begin(), token.end(), str.begin(),
                                   [] (char c1, char c2) { return c1 == c2; });
        }

        ///
        /// \brief check if a string starts with a token (case insensitive)
        ///
        inline bool istarts_with(const std::string& str, const std::string& token)
        {
                return  str.size() >= token.size() &&
                        std::equal(token.begin(), token.end(), str.begin(),
                                   [] (char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
        }
}
