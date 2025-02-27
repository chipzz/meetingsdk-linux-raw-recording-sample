#pragma once

#include <string>

std::string ToString(std::string &s);
std::string ToString(const char *const s);
std::string ToString(int i);

std::string ToJSONString(std::string &s);
std::string ToJSONString(const char *const s);
std::string ToJSONString(unsigned long i);
std::string ToJSONString(unsigned int i);
std::string ToJSONString(int i);
std::string ToJSONString(bool b);
