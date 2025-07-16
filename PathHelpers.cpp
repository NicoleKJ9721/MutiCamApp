#include "PathHelpers.h"
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace fs = std::filesystem;

std::string get_executable_dir() {
    #ifdef _WIN32
        char path[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return fs::path(path).parent_path().string();
    #elif defined(__linux__)
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return fs::path(std::string(result, (count > 0) ? count : 0)).parent_path().string();
    #elif defined(__APPLE__)
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0)
            return fs::path(path).parent_path().string();
    #endif
    return ""; // or handle error
} 