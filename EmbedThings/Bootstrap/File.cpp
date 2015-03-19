//
//  File.cpp
//  ccubed-sample
//
//  Created by Andreia Gaita on 5/9/14.
//  Copyright (c) 2014 Andreia Gaita. All rights reserved.
//

#include "File.h"
#include <cstdarg>

std::string File::ExecDir;
char File::PathSeparator;

void File::SetExecDir(const char* path) {
	std::string::size_type n;
	std::string s(path);
	PathSeparator = '/';
	n = s.rfind(PathSeparator);
	if (n == std::string::npos) {
		PathSeparator = '\\';
		n = s.rfind(PathSeparator);
	}
	if (n < s.length())
		ExecDir = s.substr(0, s.length()) + PathSeparator;
	//if (n == std::string::npos)
		//ExecDir = std::move(s);
	else
		ExecDir = s.substr(0, n);
}

std::string File::Read(std::string filename) {
	//std::ifstream file(ExecDir + PathSeparator + filename);
	std::ifstream file(filename.c_str());
	if (file) {
		// get length of file:
		file.seekg(0, file.end);
		auto length = file.tellg();
		file.seekg(0, file.beg);

		std::string str;
		str.resize(length); // reserve space
		file.read(&str[0], length);
		file.close();
		
		return str;
	}
	return std::string();
}

char* File::Read(std::string filename, size_t* len) {
	//std::ifstream file(ExecDir + PathSeparator + filename);
	std::ifstream file(filename.c_str(), std::ifstream::binary);
	if (file) {
		// get length of file:
		file.seekg(0, file.end);
		auto length = file.tellg();
		*len = length;
		file.seekg(0, file.beg);

		char* arr = new char[length];
		file.read(arr, length);
		file.close();

		return arr;
	}
	return nullptr;

}

