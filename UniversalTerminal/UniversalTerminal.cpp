// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "CMakeProject1.h"
#include <cstdio>

#ifdef linux
#include <unistd.h>
#elif defined _WIN32
#include <windows.h>
#endif

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
		printf("%s > ", cwd);
		
		scanf("%s", comand);
		execute(comand);
	}

	return 0;
}
