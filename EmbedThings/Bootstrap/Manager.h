#pragma once

#include "stdafx.h"
#include "Application.h"
class Manager {
public:
	static void RegisterApplication(Application* app);
	static Application* GetApplication();
private:
	static Application* application;
};