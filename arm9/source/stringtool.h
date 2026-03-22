/*
    Copyright (C) 2007 Acekard, www.acekard.com

    SPDX-License-Identifier: MIT
*/

#pragma once

#include <string>

std::string formatString(const char* fmt, ...);

bool startsWithString(const std::string& s, const std::string& prefix);

bool endsWithString(const std::string& s, const std::string& suffix);

bool containsString(const std::string& str, const std::string& substr);

std::string replaceInString(const std::string& s, const std::string& oldValue, const std::string& newValue);

std::string toLowerString(const std::string& value);

int linesInString(const std::string& value);