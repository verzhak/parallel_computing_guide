
#ifndef ALL_H
#define ALL_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mpi.h>

#define ERROR \
{\
	fprintf(stderr, "[Исключение] Файл %s, строка %d, код ошибки %d\n", __FILE__, __LINE__, ret);\
	return -1;\
};

#endif

