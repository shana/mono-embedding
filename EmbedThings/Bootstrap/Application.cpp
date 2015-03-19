#include "File.h"
#include "Application.h"

#include <mono/metadata/mono-config.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-gc.h>

#include <vector>
#include <chrono>


void Application::StartMonoThread()
{
	// this will override the internal assembly search logic.
	// do it in case you package mono in a different structure
	//mono_set_assemblies_path(dllDir.c_str());

	mono_config_parse(NULL);
	domain = mono_jit_init_version("CCubed Root Domain", "v4.0.30319");

	// soft debugger needs this
	mono_thread_set_main(mono_thread_current());

	Run();

	while (true) {
		char c;
		std::cout << "Press r to recompile";
		std::cin >> c;

		if (c == 'r') {
			for (const auto& img : images) {

			}
			MonoClass* klass = mono_class_from_name(image, "Embed", "Test");
			if (klass == nullptr) {
				printf_console("Failed loading class %s\n", "Embed.Test");
				return;
			}

		}
/*
		auto start = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::seconds(2));
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		printf_console("Waited %i ms\n", elapsed.count());
*/
	}
	
}


void
Application::Init()
{
	assemblyDir = "Managed";

	std::vector<std::string> monoPaths;
	std::string monoDir = File::BuildRootedPath("mono");
	std::string libmonoDir = File::BuildPath(monoDir, "lib");
	std::string etcmonoDir = File::BuildPath(monoDir, "etc");

	std::string dllDir = File::BuildRootedPath(assemblyDir);

	mono_set_dirs(libmonoDir.c_str(), etcmonoDir.c_str());

	monoPaths.push_back(dllDir.c_str());

	//std::thread t(&Application::StartMono, this);
	//monoThread = std::move(t);
	StartMonoThread();
}


void
Application::Run()
{
	RestartMono();

	bool run = true;

}

void UnloadDomain()
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

MonoDomain* LoadDomain()
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

void
Application::StartMono()
{
	domain = LoadDomain();
	if (!domain) {
		printf_console("Error loading domain\n");
		return;
	}
}

void
Application::StopMono()
{
	for (auto& img : images)
		mono_image_close(img);
	images.clear();
	UnloadDomain();

}

void Application::RestartMono()
{
	StopMono();
	StartMono();

	std::string dll = "test.dll";
	std::string filename = File::BuildRootedPath(assemblyDir, dll);
	size_t length;
	char* data = File::Read(filename.c_str(), &length);

	printf_console("Loading %s into Domain\n", dll.c_str());

	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open_from_data_with_name(data, length, true /* copy data */, &status, false /* ref only */, filename.c_str());
	if (status != MONO_IMAGE_OK || image == nullptr)
	{
		printf_console("Failed loading assembly %s\n", dll);
		return;
	}

	MonoAssembly* assembly = mono_assembly_load_from_full(image, filename.c_str(), &status, false);
	if (status != MONO_IMAGE_OK || assembly == nullptr)
	{
		mono_image_close(image);
		printf_console("Failed loading assembly %s\n", dll);
		return;
	}

	images.push_back(image);

	MonoClass* klass = mono_class_from_name(image, "Embed", "Test");
	if (klass == nullptr) {
		printf_console("Failed loading class %s\n", "Embed.Test");
		return;
	}
	MonoObject* obj = mono_object_new(mono_domain_get(), klass);
	if (obj == nullptr) {
		printf_console("Failed loading class instance %s\n", "Embed.Test");
		return;
	}
	mono_runtime_object_init(obj);
	if (obj == nullptr) {
		printf_console("Failed initializing class instance %s\n", "Embed.Test");
		return;
	}

}