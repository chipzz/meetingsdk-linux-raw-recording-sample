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

template<class V> std::string ToJSONKeyValueString(const char *const key, V value)
{
	return ToJSONString(key) + ": " + ToJSONString(value);
}

template<class V, class ...R> std::string ToJSONKeyValueString(const char *const key, V value, R ...rest)
{
	return ToJSONKeyValueString(key, value) + ", " + ToJSONKeyValueString(rest...);
}

template<class ...R> std::string ToJSONString(const char *const key, R ...rest)
{
	return "{ " + ToJSONKeyValueString(key, rest...) + " }";
}
