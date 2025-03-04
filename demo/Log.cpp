#include "Log.h"

std::string ToString(const char *const s)
{
	return '"' + std::string(s) + '"';
}

std::string ToString(const std::string &s)
{
	return ToString(s.c_str());
}

std::string ToString(int i)
{
	return std::to_string(i);
}

std::string ToString(bool b)
{
	return b ? "true" : "false";
}

std::string ToJSONString(const char *const s)
{
	return '"' + std::string(s) + '"';
}

std::string ToJSONString(const std::string &s)
{
	return ToJSONString(s.c_str());
}

std::string ToJSONString(unsigned long i)
{
	return std::to_string(i);
}

std::string ToJSONString(unsigned int i)
{
	return std::to_string(i);
}

std::string ToJSONString(int i)
{
	return std::to_string(i);
}

std::string ToJSONString(bool b)
{
	return ToString(b);
}

void CallbackLog::print(std::ostream &s) const
{
	std::chrono::milliseconds::rep const c = std::chrono::duration_cast<std::chrono::microseconds>(start_time.time_since_epoch()).count();
	s << (c / 1000000) << "." << (c % 1000000) << " ";
	s << file << ':' << line << '\t';
	if (singleton)
		s << singleton << '.';
	if (function)
		s << function;
	s << "(";
	bool first = true;
//	for (const std::string &a : json_args)
	for (const std::string &a : string_args)
	{
		if (first)
			first = false;
		else
			s << ", ";
		s << a;
	}
	s << ")";
	s << std::endl;
}

CallbackLog::CallbackLog
(
	const char *const file,
	const unsigned int line,
	const char *const singleton,
	const char *const function
) :
	start_time(std::chrono::system_clock::now()),
	file(file + CMAKE_SOURCE_DIR_LENGTH),
	line(line),
	singleton(singleton),
	function(function)
{
}

CallbackLog::~CallbackLog()
{
}
