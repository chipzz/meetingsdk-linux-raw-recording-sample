#include "Serialisation.h"
#include <vector>
#include <iostream>

struct CallbackLog
{
	const char *const file;
	const unsigned int line;
	const char *const singleton;
	const char *const function;
	std::vector<std::string> string_args;
	std::vector<std::string> json_args;

	void print(std::ostream &s) const
	{
		s << file << ':' << line << '\t';
		if (singleton)
			s << singleton << '.';
		if (function)
			s << function;
		s << "(";
		bool first = true;
//		for (const std::string &a : json_args)
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

	CallbackLog
	(
		const char *const file,
		const unsigned int line,
		const char *const singleton,
		const char *const function
	) :
		file(file + 48),
		line(line),
		singleton(singleton),
		function(function)
	{
	}

	template <class F, class ...R> CallbackLog
	(
		const char *const file,
		const unsigned int line,
		const char *const singleton,
		const char *const function,
		F &f,
		R ...rest
	) :
		CallbackLog(file, line, singleton, function, rest...)
	{
		string_args.insert(string_args.begin(), ToString(f));
		json_args.insert(json_args.begin(), ToJSONString(f));
	}

	~CallbackLog()
	{
	}
};

#define Q(x) #x
#define QUOTE(x) Q(x)
#define LOG_CALLBACK_(s, f, ...) CallbackLog v(__FILE__, __LINE__, s, f __VA_OPT__(,) __VA_ARGS__);
#define LOG_CALLBACK(s, f, ...) LOG_CALLBACK_(s, f, __VA_ARGS__); v.print(std::cerr);
