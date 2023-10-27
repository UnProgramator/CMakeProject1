#pragma once

#ifdef linux
#include <unistd.h>
#include <sys/wait.h>
#elif defined _WIN32
#include <windows.h>
#endif