#pragma once


#define dirNameBuffSize 1000
#define commandBuffSize 1000

extern void reduce(char* src, int prev = 1);
extern void convert(char* src, char from, char to);

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


#ifdef _WIN32
inline void getWindowsErrorMsg(char* msgBuff, const DWORD errorCode) {
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msgBuff,
		0, NULL);
}
#endif

