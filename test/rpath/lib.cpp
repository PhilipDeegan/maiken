extern "C"
#if defined(_WIN32)
    __declspec(dllexport)
#endif
    int rpath_value() {
  return 42;
}
