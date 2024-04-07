#include "UniversalTerminal.h"
#include "utilities.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <cstdint>


void reduce(char* src, int prev) {
	if (prev == 0) return;
	int idx1 = -1, idx2 = -1;
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



void convert(char* src, char from, char to) {
	assert(from != '\0');
	while (*src != '\0') {
		if (*src == from)
			*src = to;
		src++;
	}
}

