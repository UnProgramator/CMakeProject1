#pragma once

#define dirNameBuffSize 1000
#define commandBuffSize 1000
extern void getCurrentFolder(char* cwd, int buffsize);
extern void reduce(char* src, int prev = 1);
extern void convert(char* src, char from, char to);



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