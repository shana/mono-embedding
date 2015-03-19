//
//  File.h
//  ccubed-sample
//
//  Created by Andreia Gaita on 5/9/14.
//  Copyright (c) 2014 Andreia Gaita. All rights reserved.
//

#ifndef __FILE_H__
#define __FILE_H__

#include "stdafx.h"
#include <string>

class File {
public:
	static std::string Read(std::string );
	static char* Read(std::string filename, size_t* len);

	static void SetExecDir(const char* path);
    
    static std::string GetExecDir() { return ExecDir; }

	static char PathSeparator;

	static std::string BuildPath(std::string v) {
		return v;
	}

	template<typename... Args>
	static std::string BuildPath(std::string first, Args... args) {
		return first + PathSeparator + BuildPath(args...);
	}

	static std::string BuildRootedPath(std::string v) {
		return ExecDir + v;
	}

	template<typename... Args>
	static std::string BuildRootedPath(std::string first, Args... args) {
		return ExecDir + first + PathSeparator + BuildPath(args...);
	}

private:
	static std::string ExecDir;
};

#endif /* defined(__FILE_H__) */
