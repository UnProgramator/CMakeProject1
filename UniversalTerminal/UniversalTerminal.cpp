// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "UniversalTerminal.h"
#include <cstdio>
#include <cassert>

#ifdef linux
#include <unistd.h>
#elif defined _WIN32
#include <windows.h>
#endif

void convert(char* src, char from, char to) {
	assert(from != '\0');
	while (*src != '\0') {
		if (*src == from)
			*src = to;
		src++;
	}
}

void reduce(char* src, int prev = 1) {
	if (prev == 0) return;
	int idx1=-1, idx2=-1;
	for (int i = 0; src[i] != '\0'; i++) {
		if (src[i] == '/') {
			if (idx1 == -1)
				idx1 = i;
			idx2 = i;
		}
	}

	if (idx1 != idx2)
		if (idx2 > idx1 + 3) {
			//coverts the substring from first / to the last / to ..
			strcpy(src + idx1 + 3, src + idx2);
			*(int16_t*)(void*)(src + idx1 + 1) = 0x2E2E; //sets ".."
		}
		else { //no place for more than 1 .
			src[idx1 + 1] = '.';
			if (idx2 > idx1 + 2) 
				src[idx1 + 2] = '.';
		}
}

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

void execute(char* comand) {
	printf("%s\n", comand);
}

int main()

{
	char cwd[1000];
	char comand[1000];

	while (1) {
		getCurrentFolder(cwd, sizeof(cwd));
		printf("%s > \n", cwd);
		reduce(cwd);
		printf("%s > ", cwd);
		
		scanf("%s", comand);
		execute(comand);
	}

	return 0;
}
