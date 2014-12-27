#pragma once
#include "wx/vector.h"
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
	MyException(const char str[], Types type) :
		std::runtime_error(str), type(type) {}
	MyException(const std::wstring str, Types type) :
		std::runtime_error(std::string(str.begin(), str.end()).c_str()),
			type(type) {}
};

template<typename T>
int Find(const wxVector<T> & v, const T & t)
{
	for (int i = 0; i < v.size(); i++)
	{
		if (v[i] == t)
			return i;
	}	
	return -1;
}
