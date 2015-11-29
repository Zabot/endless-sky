/* DataWriter.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DATA_WRITER_H_
#define DATA_WRITER_H_

#include <string>
#include <sstream>

class DataNode;



// This class writes data in a hierarchical format, where an indented line is
// considered the "child" of the first line above it that is less indented. By
// using this class, you can have a function add data to the file without having
// to tell that function what indentation level it is at. This class also
// automatically adds quotation marks around strings if they contain whitespace.
class DataWriter {
public:
  template <class ...B>
	void Write(const char *a, B... others);
  template <class ...B>
	void Write(const std::string &a, B... others);
  template <class A, class ...B>
	void Write(const A &a, B... others);
	void Write(const DataNode &node);
	
	virtual void Write() = 0;
	
	virtual void BeginChild() = 0;
	virtual void EndChild() = 0;
	
	virtual void WriteComment(const std::string &str) = 0;
	
	
protected:
	virtual void AppendToken(const char *a) = 0;
	
	
private:
	void WriteToken(const char *a);
};



template <class ...B>
void DataWriter::Write(const char *a, B... others)
{
	WriteToken(a);
	Write(others...);
}



template <class ...B>
void DataWriter::Write(const std::string &a, B... others)
{
	Write(a.c_str(), others...);
}



template <class A, class ...B>
void DataWriter::Write(const A &a, B... others)
{
	static_assert(std::is_arithmetic<A>::value,
		"DataWriter cannot output anything but strings and arithmetic types.");
		
	std::ostringstream out;
	out << a;
	Write(out.str().c_str(), others...);
}



#endif
