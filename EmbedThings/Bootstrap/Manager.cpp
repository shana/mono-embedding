#include "Manager.h"

Application* Manager::application;

Application* Manager::GetApplication()
{
	return application;
}

void Manager::RegisterApplication(Application* app)
{
	application = app;
}