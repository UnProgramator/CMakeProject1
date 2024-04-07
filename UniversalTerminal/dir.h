#pragma once 

#define MAX_DIR_STACK_CHR 1000

//utitlity - get curent directory
extern void getCurrentFolder(char* cwd, int buffsize);

//pwd command
extern int  pwd(char* cmd, char**&);

//change directory commands - cd, pushd, popd
extern int  cd(char* cmd, char**&);
extern int  pushd(char* cmd, char**&);
extern int  popd(char* cmd, char**&);

#ifdef _WIN32
extern int win_ls(char* cmd, char**&);
#endif