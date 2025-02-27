#pragma once

#include <string>

std::string ToString(const std::string &);
std::string ToString(const char *const);
std::string ToString(int);

std::string ToJSONString(const std::string &);
std::string ToJSONString(const char *const);
std::string ToJSONString(unsigned long);
std::string ToJSONString(unsigned int);
std::string ToJSONString(int);
std::string ToJSONString(bool);

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
