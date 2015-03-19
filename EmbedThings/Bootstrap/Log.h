#if WINDOWS
void __cdecl odprintf(const char *format, ...);
#define printf_console odprintf
#else
#define printf_console printf
#endif
