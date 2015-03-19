#pragma once

#include "stdafx.h"
#include <mono/jit/jit.h>
#include <thread>
#include <vector>

class Application
{
public:
	void Init();
	void InitializeMono();
	void Run();
	bool StartMonoAndLoadAssemblies();
	void StopMono();
	bool StartMono();
	void FireOnReload();

private:
	std::string assemblyDir;
	MonoDomain* domain;
	std::vector<MonoImage*> images;
	std::vector<MonoObject*> instances;
};
