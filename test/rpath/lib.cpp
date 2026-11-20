#if defined(_WIN32)
__declspec(dllexport)
#endif
extern "C" int rpath_value() { return 42; }
