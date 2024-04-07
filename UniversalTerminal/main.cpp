// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "UniversalTerminal.h"
#include "execute.h"
#include "utilities.h"
#include "dir.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#ifdef linux
#include <unistd.h>
#include <sys/wait.h>
#elif defined _WIN32
#include <windows.h>
#endif



int main(int argc, char** argv, char** envp)

{
	char cwd[dirNameBuffSize];
	char comand[commandBuffSize];

	char* var;
	var = envp[0];
	int i = 0;
	/*while (var) {
		printf("%s\n", var);
		i++;
		var = envp[i];
	}*/

	while (1) {
		getCurrentFolder(cwd, sizeof(cwd));

		//printf("%s > \n", cwd);
		reduce(cwd);
		printf(KGRN "%s" KWHT " > ",cwd);
		
		fgets(comand, sizeof(cwd), stdin);
		execute(comand, envp);
	}

	return 0;
}
