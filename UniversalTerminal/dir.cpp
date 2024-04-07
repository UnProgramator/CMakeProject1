#include "dir.h"
#include "UniversalTerminal.h"
#include "utilities.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>

#ifdef _WIN32
#include <strsafe.h>
#endif

static char cwd[dirNameBuffSize];

void getCurrentFolder(char* cwd, int buffsize) {
#ifdef linux
	if (getcwd(cwd, buffsize) == nullptr) {
		printf("Error occured");
		exit(-1);
	}
#elif defined _WIN32
	auto retval = GetCurrentDirectory(buffsize, cwd);
	if (retval == 0) {
		printf("Error occured");
		exit(-1);
	}
	convert(cwd, '\\', '/');
#endif
}


#ifdef _WIN32

static int _cd(char* ndir) {
	errno = 0; // reset errno

	if (ndir[1] != ':') { // path is relative to the curent directory, not somehing like C:/
		getCurrentFolder(cwd, sizeof(cwd));

		size_t len1 = strlen(cwd);
		cwd[len1] = '/';
		cwd[++len1] = '\0';

		strncat(cwd, ndir, dirNameBuffSize - len1);
		ndir = cwd;
	}

	if (!SetCurrentDirectory(ndir)) {
		char lpMsgBuf[500];
		DWORD msgErrorCod = GetLastError();
		getWindowsErrorMsg(lpMsgBuf, msgErrorCod);

		printf("Error on \"cd %s\" (code %lu): %s\n", ndir, msgErrorCod, lpMsgBuf);
		return msgErrorCod;
	}

	cwd[0] = '\0';
	return 0;
}

#elif defined linux

static int _cd(char* ndir) {
	errno = 0; // reset errno

	if (ndir[0] != '/') { // path is relative to the curent directory, not full path - which starts with /
		getCurrentFolder(cwd, sizeof(cwd));

		int len1 = strlen(cwd);
		cwd[len1] = '/';
		cwd[++len1] = '\0';

		strncat(cwd, ndir, dirNameBuffSize - len1);
		ndir = cwd;
	}

	if (chdir(ndir)) {
		char buff[400];
		perror(buff);
		printf("Error on \"cd %s\" (code %d): %s\n", ndir, errno, buff);
		return errno;
	}

	cwd[0] = '\0';
	return 0;
}

#endif

int cd(char* cmd, char**&) {
	int idx2 = 3; //get over cd; already trimmed
	while (isspace(cmd[idx2])) idx2++;

	cmd += idx2; //remove the cd keyword
	return _cd(cmd); // call the inline cd function
}

int pwd(char*, char**&) {
	getCurrentFolder(cwd, sizeof(cwd));
	printf("%s\n", cwd);
	cwd[0] = '\0';
	return 0;
}


struct{
	char stack[MAX_DIR_STACK_CHR] = "\0";
	unsigned spointer = MAX_DIR_STACK_CHR-1; //stack pointer - start of last dir

	void printStack() {
		for (int i = spointer; i < MAX_DIR_STACK_CHR - 1; i++)
			if (stack[i])
				putc(stack[i], stdout);
			else
				putc(' ', stdout);
	}
} dstack;

int pushd(char* cmd, char**&) {
	int idx2 = 6; //get over cd; already trimmed
	while (isspace(cmd[idx2])) idx2++; // trim if multiple spaces after pushd
	cmd = cmd + idx2;

	//push value on directory stack
	getCurrentFolder(cwd, sizeof(cwd));

	unsigned len = (unsigned)strlen(cwd) + 1; // also include the \0

	if (dstack.spointer - len < 0) { // no space on the stack
		printf("directory stack is full");
		cwd[0] = '\0';
		return 2;
	}

	auto lastPointer = dstack.spointer;

	memcpy(dstack.stack + dstack.spointer - len, cwd, len); // copy the old directory and the \0
	dstack.spointer -= len; // set the curent directoy stack pointer

	if (_cd(cmd) != 0) { // changed directory fail
		dstack.spointer = lastPointer;
		printf("%s: no such file or directory", cmd);
		cwd[0] = '\0';
		return 1;
	}

	//print the stack for success
	dstack.printStack();
	putc('\n', stdout);
	cwd[0] = '\0';
	return 0;
}

int popd(char* cmd, char**&) {
	if (dstack.spointer == MAX_DIR_STACK_CHR) { // nothing in directory stack
		printf("Directory stack is empty");
		return 1;
	}

	if (_cd(dstack.stack + dstack.spointer) != 0) {
		printf("Could not change directory to the previous destination");
		return 2;
	}

	// set the pointer to the next avaliable slot to the start of the curent slot used, then set it to 0
	while (dstack.stack[dstack.spointer] != 0 && dstack.spointer < MAX_DIR_STACK_CHR-1) dstack.spointer++;
	dstack.spointer++; // move to next non-null character or to the end of the stack
	
	//print the rest of the stack for success
	dstack.printStack();
	putc('\n', stdout);

	return 0;
}



#ifdef _WIN32

int win_ls(char* cmd, char**&) {
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
			printf(TEXT("<D> %10lld %s%s\n%s"), (long long)0, KCYN, ffd.cFileName, KWHT);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			printf(TEXT("<F> %10lld %s bytes\n"), filesize.QuadPart, ffd.cFileName);
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