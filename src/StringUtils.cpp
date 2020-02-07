#include "StringUtils.h"
#include <AK/StringBuilder.h>
#include <regex>

bool potentially_contains_variable(const String& haystack)
{
    for (size_t i = 0; i < haystack.length(); ++i) {
        char c = haystack.characters()[i];
        if (c == '$')
            return true;
    }
    return false;
}

String replace_variables(const String& haystack, const String& varname, const String& replacement)
{
    StringBuilder sb;
    sb.append("\\$\\{");
    sb.append(varname);
    sb.append("\\}");
    std::string hs(haystack.characters(), haystack.length());
    std::string replaced = std::regex_replace(hs, std::regex(sb.build().characters()), replacement.characters());
    return replaced.c_str();
}
