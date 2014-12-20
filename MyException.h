#pragma once
#include <stdexcept>
#include <string>

class MyException : public std::runtime_error
{
public:
	enum Types
	{
		FATAL_ERROR, //quit
		NOT_FATAL //don't quit
	} type;
	MyException(const std::string str, Types type) :
		std::runtime_error(str), type(type) {}
};
