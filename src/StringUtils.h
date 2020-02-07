#pragma once

#include <AK/String.h>

bool potentially_contains_variable(const String& haystack);
String replace_variables(const String& haystack, const String& varname, const String& replacement);
