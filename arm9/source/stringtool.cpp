/*
    Copyright (C) 2007 Acekard, www.acekard.com

    SPDX-License-Identifier: MIT
*/

#include "stringtool.h"
#include <malloc.h>
#include <cstdarg>
#include <cstdio>
#include <algorithm>

std::string formatString(const char* fmt, ...) {
    const char* f = fmt;
    va_list argList;
    va_start(argList, fmt);
    char* ptempStr = NULL;
    size_t max_len = vasiprintf(&ptempStr, f, argList);
    std::string str(ptempStr);
    str.resize(max_len);
    free(ptempStr);
    va_end(argList);
    return str;
}

bool startsWithString(const std::string& s, const std::string& prefix) {
    return s.length() >= prefix.length() &&
           s.compare(0, prefix.length(), prefix) == 0;
}

bool endsWithString(const std::string& s, const std::string& suffix) {
    if (s.length() < suffix.length()) {
        return false;
    }

    return s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool containsString(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

std::string replaceInString(const std::string& s, const std::string& oldValue, const std::string& newValue) {
    std::string res = s;
    size_t pos = 0;

    while ((pos = res.find(oldValue, pos)) != std::string::npos) {
        res.replace(pos, oldValue.length(), newValue);
        pos += newValue.length();
    }

    return res;
}

std::string toLowerString(const std::string& value) {
    std::string res(value);

    for (size_t i = 0; i < res.size(); i++) res[i] = tolower(res[i]);

    return res;
}

int linesInString(const std::string& value) {
    if (value.empty()) return 0;

    return std::count(value.begin(), value.end(), '\n') + 1;
}