#include "UniversalTerminal.h"
#include "execute.h"
#include "utilities.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <map>
#include <string>
#include <initializer_list>

#ifdef _WIN32
#include <windows.h>
#include <strsafe.h>
#include <process.h>
#endif

struct str_comp {
	bool operator()(const char* a, const char* b) const {
		return strcmp(a, b) < 0;
	}
};
typedef int (*builtin_function)(char*, char**&);
typedef std::map<const char* const, builtin_function, str_comp> bi_f_type;

static int cd(char* cmd, char**&);
static int pwd(char* cmd, char**&);
extern int createProcess(char* cmd);
#ifdef _WIN32
static int win_ls(char* cmd, char**&);
#endif

bi_f_type builtin = {	{"exit", [](char*, char**&)->int { exit(0); return 0; }},
						{"cd", cd},
						{"pwd",  pwd}
#ifdef _WIN32
						,
						{"ls", win_ls}
#endif
					};

int execute(const char* const command, char** envp) {

	char* _cmd;
	{
		int s = 0, f = 0;
		while (isspace(command[s])) s++; //trim at left

		// f = strlen(command)
		f = s;
		while (command[f]) f++;

		while (isspace(command[f - 1])) f--; //trim at right

		//allocate space for a copy of the current command
		_cmd = new char[f - s + 1];
		memcpy(_cmd, command + s, f - s);
		_cmd[f - s] = '\0';
		printf("\"%s\"\n", _cmd);
	}

	{
		bi_f_type::iterator fun;
		int idx = (int)strchr(_cmd, ' ');
		if (idx != 0) {
			idx -= (int)_cmd;
			char* pcmd = new char[idx + 1]; //contains the command name
			memcpy(pcmd, _cmd, idx + 1);
			fun = builtin.find(pcmd);
			delete pcmd;
		}
		else
			fun = builtin.find(_cmd);

		if (fun != builtin.end()) {
			fun->second(_cmd, envp);
			return 0;
		}
	}

#if defined linux
	int child = fork();
	if (child == 0) {
		char* prog[20] = { _cmd };
		{
			int i = 0;
			for (prog[i] = strtok(_cmd, " \t\n\r"); prog[i]; prog[++i] = strtok(nullptr, " \t\n\r"));
			printf("execute \"%s\" with %d arguments\n", command, i);
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
		delete _cmd;
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
	/*if (execve(prog[0], prog, envp) != 0) {
		char s[500];
		perror(s);
		printf("execl comand failed with message: %s\n", s);
		exit(errno);
	}*/
#endif
	return 1;
}

int createProcess(char* cmd)
{
#if defined linux
	if (chdir(cmd)) {
		char buff[400];
		perror(buff);
		printf("Error on \"cd %s\" (code %d): %s\n", cwd, errno, buff);
		return errno;
	}
#elif defined _WIN32
	//convert(cwd, '/', '\\'); // works without
	if (!SetCurrentDirectory(cmd)) {
		char lpMsgBuf[500];
		DWORD msgErrorCod;
		getWindowsErrorMsg(lpMsgBuf, msgErrorCod);

		printf("Error on \"cd %s\" (code %lu): %s\n", cmd, msgErrorCod, lpMsgBuf);
		return msgErrorCod;
	}
#endif
	return 0;
}


static char cwd[dirNameBuffSize];

static int cd(char* cmd, char**&) {
	getCurrentFolder(cwd, sizeof(cwd));

	int len1 = strlen(cwd);
	cwd[len1] = '/';

	cwd[++len1] = '\0';
	int idx2 = 2; //get over cd; already trimmed
	while (isspace(cmd[idx2])) idx2++;
	strncat(cwd, cmd + idx2, dirNameBuffSize - len1 - 1);

	int retval = createProcess(cwd);

	cwd[0] = '\0';
	return retval;
}



static int pwd(char*, char**&) {
	getCurrentFolder(cwd, sizeof(cwd));
	printf("%s\n", cwd);
	cwd[0] = '\0';
	return 0;
}
#ifdef _WIN32
static int win_ls(char* cmd, char**&) {
	char* prog[20] = { cmd };
	{
		int i = 0;
		for (prog[i] = strtok(cmd, " \t\n\r"); prog[i]; prog[++i] = strtok(nullptr, " \t\n\r"));
	}
	getCurrentFolder(cwd, sizeof(cwd));


	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	LARGE_INTEGER filesize;

	// Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

	StringCchCopy(szDir, MAX_PATH, cwd);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	// Find the first file in the directory.

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		char lpMsgBuf[500];
		DWORD msgErrorCod = GetLastError();
		getWindowsErrorMsg(lpMsgBuf, msgErrorCod);
		lpMsgBuf[499] = '\0';
		printf("Error %ld with message: \"%s\"", msgErrorCod, lpMsgBuf);
		cwd[0] = '\0';
		return msgErrorCod;
	}

	// List all the files in the directory with some info about them.

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			printf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			printf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		char lpMsgBuf[500];
		getWindowsErrorMsg(lpMsgBuf, dwError);
		lpMsgBuf[499] = '\0';
		printf("Error %ld with message: \"%s\"", dwError, lpMsgBuf);
		cwd[0] = '\0';
		return dwError;
	}

	FindClose(hFind);
	cwd[0] = '\0';
	return dwError;
}
#endif