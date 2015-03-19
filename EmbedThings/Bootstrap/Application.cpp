#include "File.h"
#include "Manager.h"

#include <mono/metadata/mono-config.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/environment.h>

#include <vector>
#include <chrono>

static void reload()
{
	printf_console("Reloading\n");
	auto app = Manager::GetApplication();
	app->FireOnReload();
}

MonoMethod* find_method(MonoClass* klass, const char* name)
{
	MonoMethod* method = mono_class_get_method_from_name(klass, name, -1);
	if (!method) return NULL;

	return method;
}

MonoMethod* find_method(MonoImage* image, const char* className, const char* namspace, const char* methodName)
{
	MonoClass* klass = mono_class_from_name(image, namspace, className);
	if (!klass)	return NULL;

	return find_method(klass, methodName);
}

MonoDomain* load_domain()
{
	MonoDomain* newDomain = mono_domain_create_appdomain("CCubed Child Domain", NULL);
	if (!newDomain) {
		printf("Error creating domain\n");
		return nullptr;
	}

	mono_thread_push_appdomain_ref(newDomain);

	if (!mono_domain_set(newDomain, false)) {
		printf_console("Error setting domain\n");
		return nullptr;
	}

	return mono_domain_get();
}

void unload_domain()
{
	MonoDomain* old_domain = mono_domain_get();
	if (old_domain && old_domain != mono_get_root_domain()) {
		if (!mono_domain_set(mono_get_root_domain(), false))
			printf_console("Error setting domain\n");

		mono_thread_pop_appdomain_ref();
		mono_domain_unload(old_domain);
	}

	//unloading a domain is also a nice point in time to have the GC run.
	mono_gc_collect(mono_gc_max_generation());
}


void Application::Init()
{
	assemblyDir = "Managed";

	std::vector<std::string> monoPaths;
	// root of the mono dir
	std::string monoDir = File::BuildRootedPath("mono");
	std::string libmonoDir = File::BuildPath(monoDir, "lib");
	std::string etcmonoDir = File::BuildPath(monoDir, "etc");

	// where we find user code
	std::string dllDir = File::BuildRootedPath(assemblyDir);

	mono_set_dirs(libmonoDir.c_str(), etcmonoDir.c_str());

	// save it for later, although it's not used it can be useful
	// to scan for new assemblies
	monoPaths.push_back(dllDir.c_str());

	InitializeMono();
}

void Application::InitializeMono()
{
	//this will override the internal assembly search logic.
	//do it in case you package mono in a different structure
	//mono_set_assemblies_path(dllDir.c_str());

	mono_config_parse(NULL);

	// initialize the root domain which will hold corlib and will always be alive
	domain = mono_jit_init_version("CCubed Root Domain", "v4.0.30319");

	// soft debugger needs this
	mono_thread_set_main(mono_thread_current());

	// add icalls
	mono_add_internal_call("EmbedThings.EntryPoint::reload", reload);

	// run the c# bits
	Run();

	// we're exiting the whole thing, cleanup
	mono_jit_cleanup(mono_domain_get());

}

void Application::Run()
{
	// this is going to block until managed code exits
	while (!StartMonoAndLoadAssemblies()) {
		
	}

	// unload the child domain
	unload_domain();
}

bool Application::StartMonoAndLoadAssemblies()
{
	// shutdown the child domain
	StopMono();

	// create a new child domain
	if (!StartMono()) {
		mono_environment_exitcode_set(-1);
		return true;
	}


	std::string dll = "EmbedThings.dll";
	std::string filename = File::BuildRootedPath(assemblyDir, dll);
	size_t length;
	// read our entry point assembly
	char* data = File::Read(filename.c_str(), &length);

	printf_console("Loading %s into Domain\n", dll.c_str());

	MonoImageOpenStatus status;
	// open the assembly from the data we read, so we never lock files
	auto image = mono_image_open_from_data_with_name(data, length, true /* copy data */, &status, false /* ref only */, filename.c_str());
	if (status != MONO_IMAGE_OK || image == nullptr)
	{
		printf_console("Failed loading assembly %s\n", dll);
		return true;
	}

	// load the assembly
	auto  assembly = mono_assembly_load_from_full(image, filename.c_str(), &status, false);
	if (status != MONO_IMAGE_OK || assembly == nullptr)
	{
		mono_image_close(image);
		printf_console("Failed loading assembly %s\n", dll);
		return true;
	}

	// save the image for lookups later and for cleaning up
	images.push_back(image);

	if (!assembly) {
		printf_console("Couldn't find assembly %s\n", filename.c_str());
		return true;
	}

	// locate the class we want to load
	MonoClass* klass = mono_class_from_name(image, "EmbedThings", "EntryPoint");
	if (klass == nullptr) {
		printf_console("Failed loading class %s\n", "EmbedThings.EntryPoint");
		return true;
	}

	// create the class (doesn't run constructors)
	MonoObject* obj = mono_object_new(mono_domain_get(), klass);
	if (obj == nullptr) {
		printf_console("Failed loading class instance %s\n", "EmbedThings.EntryPoint");
		return true;
	}

	// initialize the class instance (runs default constructors)
	mono_runtime_object_init(obj);
	if (obj == nullptr) {
		printf_console("Failed initializing class instance %s\n", "EmbedThings.EntryPoint");
		return true;
	}

	// save the class instance for lookups later
	instances.push_back(obj);

	// find the Run() method
	auto method = find_method(klass, "Run");
	MonoObject *result, *exception;

	// call the Run method. This will block until the managed code decides to exit
	result = mono_runtime_invoke(method, obj, NULL, NULL);
	int val = *(int*)mono_object_unbox(result);

	// if the managed code returns with 0, it wants to exit completely
	if (val == 0) {
		return true;
	}
	return false;
}

bool Application::StartMono()
{
	domain = load_domain();
	if (!domain) {
		printf_console("Error loading domain\n");
		return false;
	}
	return true;
}

void Application::StopMono()
{
	for (auto& img : images)
		mono_image_close(img);
	instances.clear();
	images.clear();
	unload_domain();

}

void Application::FireOnReload()
{
	for (auto& obj : instances) {
		auto klass = mono_object_get_class(obj);
		if (klass) {
			auto method = find_method(klass, "OnReload");
			if (method)
				mono_runtime_invoke(method, obj, NULL, NULL);
		}
	}
}
