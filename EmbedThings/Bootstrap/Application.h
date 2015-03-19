#pragma once

#include "stdafx.h"
#include <mono/jit/jit.h>
#include <thread>

class Application
{
public:
	void Init();
	void Run();
	void RestartMono();
	void StopMono();
	void StartMono();
	void StartMonoThread();

private:
	std::string assemblyDir;
	MonoDomain* domain;
	std::thread monoThread;
	std::vector<MonoImage*> images;
};
