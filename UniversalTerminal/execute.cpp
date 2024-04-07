#include "UniversalTerminal.h"
#include "execute.h"
#include "utilities.h"
#include "dir.h"

// c/c++ header files
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <map>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

struct str_comp {
	bool operator()(const char* a, const char* b) const {
		return strcmp(a, b) < 0;
	}
};

typedef int (*builtin_function)(char*, char**&);
typedef std::map<const char* const, builtin_function, str_comp> bi_f_type;




bi_f_type builtin = {	{"exit", [](char*, char**&)->int { exit(0); return 0; }},
						{"cd", cd},
						{"pwd",  pwd},
						{"pushd", pushd},
						{"popd", popd}
#ifdef _WIN32
						,
						{"ls", win_ls}
#endif
					};

static char _cmd[commandBuffSize];

static int execute_buildtin(builtin_function, char*, char**&);

int execute(const char* const command, char** envp) {

	{
		int s = 0, f = 0;
		while (isspace(command[s])) s++; //trim at left

		// f = strlen(command)
		f = s;
		while (command[f]) f++;

		while (isspace(command[f - 1])) f--; //trim at right

		if (f - s > sizeof(_cmd)) {
			exit(-1);
		}

		//allocate space for a copy of the current command
		memcpy(_cmd, command + s, f - s);
		_cmd[f - s] = '\0';

#ifdef DEBUG_BUILD
		printf("\"%s\"\n", _cmd);
#endif
	}

	//check if it is a build-in
	{
		bi_f_type::iterator fun;

		char* sp_chr = strchr(_cmd, ' ');

		if (sp_chr != 0) { // space was found
			char* pcmd = new char[sp_chr - _cmd + 1]; //contains the command name
			if (!pcmd)
				exit(-1);
			memcpy(pcmd, _cmd, sp_chr - _cmd);
			pcmd[sp_chr - _cmd] = '\0';
			fun = builtin.find(pcmd);
			delete[] pcmd;
		}
		else
			fun = builtin.find(_cmd);

		//if it is a built-in, then execute
		if (fun != builtin.end())
			return execute_buildtin(fun->second,_cmd, envp);
	}

	//execute if it was not a built-in command

#if defined linux
	int child = fork();
	if (child == 0) {
		char* prog[20] = { _cmd };
		{
			int i = 0;
			for (prog[i] = strtok(_cmd, " \t\n\r"); prog[i]; prog[++i] = strtok(nullptr, " \t\n\r"));
#ifdef DEBUG_BUILD
			printf("execute \"%s\" with %d arguments\n", command, i);
#endif
		}

		if (execve(prog[0], prog, envp) != 0) {
			char s[500];
			perror(s);
			printf("execl comand failed with message: %s\n", s);
			exit(errno);
		}

	}
	else {
		int status;
		waitpid(child, &status, 0);
	}
#elif _WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcessA(nullptr, 
						_cmd, 
						nullptr,
						nullptr,
						false,
						0,
						nullptr,
						nullptr,
						&si,
						&pi
	)) {
		printf("CreateProcess failed (%d).\n", GetLastError());
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

#endif
	return 1;
}

int execute_buildtin(builtin_function fun, char* cmd, char** & envp) {
	return fun(cmd, envp);
}
