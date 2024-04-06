#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS
#elif defined(__APPLE__)
#define MACOS
#elif defined(__linux__)
#define LINUX
#endif