#include "Manager.h"
#include "File.h"

static Manager manager;

int
main(int argc, const char* argv[])
{
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);
	std::string str(pwd);

	File::SetExecDir(str.c_str());

	auto app = new Application();
	Manager::RegisterApplication(app);
	app->Init();
	return 0;
}