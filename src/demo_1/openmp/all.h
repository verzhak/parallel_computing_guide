
#ifndef ALL_H
#define ALL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <omp.h>

#define ERROR \
{\
	fprintf(stderr, "[Исключение] Файл %s, строка %d\n", __FILE__, __LINE__);\
	return -1;\
};

#endif

