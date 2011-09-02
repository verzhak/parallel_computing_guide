
/*

Многопоточное моделирование логнормального распределения с помощью датчика псевдослучайных чисел, распределенных равномерно

Сборка программы:

	gcc -Wall -lm -lrt -lOpenCL main.c -o demo

	Здесь:

		-Wall - ключ, предписывающий компилятору выводить предупреждения о всех потенциальных ошибках в программе;
		-lm - ключ, подключающий к программе математическую библиотеку из состава стандартной библиотеки языка программирования C;
		-lrt - ключ, подключающий к программе библиотеку получения информации о времени;
		-lOpenCL - ключ, подключающий к программе библиотеку - загрузчик OpenCL-драйвера;
		main.c - файл с исходным кодом программы;
		-o demo - указание имени результирующего исполняемого файла.

Запуск программы:

	./demo KERNEL_FNAME TNUM MU SIGMA N

	Здесь:

		KERNEL_FNAME - относительный или абсолютный путь и имя файла с исходным кодом ядер (главных функций) вычислительных потоков;
		TNUM - количество вычислительных потоков;
		MU, SIGMA - параметры (лог)нормального распределения;
		N - количество вычисляемых реализаций псевдослучайной величины с логнормальным распределением.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <CL/cl.h>

/* Макрос, расценивающий истинность условия condition как ошибку */
#define ERROR(condition) \
{\
	if((condition))\
	{\
		fprintf(stderr, "[Ошибка] Строка %u, Файл %s\n", __LINE__, __FILE__);\
		return -1;\
	}\
}

/* Макрос, расценивающий неравенство выражения expr значению константы компилятора CL_SUCCESS как ошибку - макрос проверяет успешность (возврат CL_SUCCESS) выполнения некоторой функции стандарта OpenCL */
#define ERROR_CL(expr) ERROR((expr) != CL_SUCCESS)

/* Функция загрузки исходного кода ядер (главных функций) вычислительных потоков */
char * kernel_load(const char * fname);

/* Функция, моделирующая логнормальное распределение */
int eval(char * src, unsigned n, float mu, float sigma, unsigned N);

/* Главная функция программы */
int main(const int argc, const char * argv[])
{
	if(argc != 6)
	{
		fprintf(stderr, "\nНедостаточно или избыток аргументов\n\n");
		return -1;
	}

	/* Загрузка исходного кода ядер (главных функций) вычислительных потоков */
	char * src = kernel_load(argv[1]);

	if(src == NULL)
	{
		fprintf(stderr, "\nОшибка при считывании исходного кода ядер\n\n");
		return -1;
	}

	/* Моделирование логнормального распределения */
	return eval(src, atoi(argv[2]), atof(argv[3]), atof(argv[4]), atoi(argv[5]));
}

/* Функция загрузки исходного кода ядер (главных функций) вычислительных потоков */
char * kernel_load(const char * fname)
{
	char * src;
	long size;
	FILE * fl = fopen(fname, "r");

	if(fl == NULL)
		return NULL;

	fseek(fl, 0, SEEK_END);
	
	/* Определение размера в байтах файла с исходным кодом */
	size = ftell(fl) + 1;
	src = malloc(size + 1);

	fseek(fl, 0, SEEK_SET);

	/* Чтение исходного кода */
	fread(src, size, 1, fl);
	src[size] = '\0';

	fclose(fl);

	return src;
}

/* Псевдослучайные числа, генерируемые функцией drand48(), равномерно распределены на диапазоне [0; 1) - функция log_normal_distrib() генерирует псевдослучайные числа с логнормальным распределением на диапазоне [-1; 1] */
float log_normal_distrib(float mu, float sigma)
{
	return exp(sigma * cos(2 * M_PI * drand48()) * sqrt(-2 * log(drand48())) + mu);
}

/* Функция, моделирующая логнормальное распределение */
int eval(char * src, unsigned n, float mu, float sigma, unsigned N)
{
	size_t st_n = n;
	unsigned u, cur_to, step = N / n, * from = malloc(n * sizeof(unsigned)), * to = malloc(n * sizeof(unsigned));
	float * X = malloc(N * sizeof(float));
	float rM, rD, * M = malloc(n  * sizeof(float)), * D = malloc(n  * sizeof(float));
	struct timespec ts_before, ts_after;

	if(
		(X == NULL)
		||
		(M == NULL)
		||
		(D == NULL)
	  )
		return -1;

	clock_gettime(CLOCK_REALTIME, & ts_before);

	/* Инициализация датчика псевдослучайных чисел */
	srand48(lrand48());

	/* Генерация N реализаций псевдослучайной величины с логнормальным распределением */
	for(u = 0; u < N; u++)
		X[u] = log_normal_distrib(mu, sigma);

	/* Расчет границ подмассивов массива X, каждый из которых будет обработан отдельным вычислительным потоком */
	for(u = 1, from[0] = 0, to[0] = step, cur_to = 2 * step; u < n; u++, cur_to += step)
	{
		from[u] = to[u - 1];
		to[u] = cur_to;
	}

	to[n - 1] = N;

	/* ############################################################################ */

	cl_int err;
	cl_uint num_dev, num_pltf;
	cl_device_id dev_id;
	cl_platform_id pltf_id;
	cl_context context;
	cl_command_queue com_queue;
	cl_program prog;
	cl_mem buf[4];
	cl_kernel M_kernel, D_kernel;

	/* Получение идентификатора первого из доступных OpenCL-драйверов */
	ERROR_CL(clGetPlatformIDs(1, & pltf_id, & num_pltf));
	ERROR(! num_pltf);

	/* Получение идентификатора первого из доступных устройств, работающих через драйвер, идентификатор которого был получен ранее */
	ERROR_CL(clGetDeviceIDs(pltf_id, CL_DEVICE_TYPE_ALL, 1, & dev_id, & num_dev));
	ERROR(! num_dev);

	/* Создание контекста вычислений */
	context = clCreateContext(NULL, 1, & dev_id, NULL, NULL, & err);
	ERROR_CL(err);

	/* Создание описателя OpenCL-программы, содержащей ядра (главные функции) вычислительных потоков */
	prog = clCreateProgramWithSource(context, 1, (const char **) & src, NULL, & err);
	ERROR_CL(err);

	/* Компиляция ядер (главных функций) вычислительных потоков */
	ERROR_CL(clBuildProgram(prog, 0, NULL, NULL, NULL, NULL));

	/* Создание трех буферов, расположенных в глобальной памяти и доступных вычислительным потокам на чтение, и одного буфера, расположенного в глобальной памяти и доступного вычислительным потокам на запись, через которые будет организован канал обмена информацией между главной программой и ядрами (главными функциями) вычислительных потоков */

	buf[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, N * sizeof(float), NULL, & err);
	ERROR_CL(err);

	buf[1] = clCreateBuffer(context, CL_MEM_READ_ONLY, n * sizeof(unsigned), NULL, & err);
	ERROR_CL(err);

	buf[2] = clCreateBuffer(context, CL_MEM_READ_ONLY, n * sizeof(unsigned), NULL, & err);
	ERROR_CL(err);

	buf[3] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, n * sizeof(float), NULL, & err);
	ERROR_CL(err);

	/* Создание очереди команд в ранее созданном контексте */
	com_queue = clCreateCommandQueue(context, dev_id, 0, & err);
	ERROR_CL(err);

	/* Получение описателя ядра M */
	M_kernel = clCreateKernel(prog, "M", & err);
	ERROR_CL(err);

	/* Получение описателя ядра D */
	D_kernel = clCreateKernel(prog, "D", & err);
	ERROR_CL(err);

	/* Определение фактических параметров ядер M и D */
	clSetKernelArg(M_kernel, 0, sizeof(cl_mem), (void *) & buf[0]);
	clSetKernelArg(D_kernel, 0, sizeof(cl_mem), (void *) & buf[0]);

	clSetKernelArg(M_kernel, 1, sizeof(unsigned), (void *) & N);
	clSetKernelArg(D_kernel, 1, sizeof(unsigned), (void *) & N);

	clSetKernelArg(M_kernel, 2, sizeof(cl_mem), (void *) & buf[1]);
	clSetKernelArg(D_kernel, 2, sizeof(cl_mem), (void *) & buf[1]);

	clSetKernelArg(M_kernel, 3, sizeof(cl_mem), (void *) & buf[2]);
	clSetKernelArg(D_kernel, 3, sizeof(cl_mem), (void *) & buf[2]);

	clSetKernelArg(M_kernel, 4, sizeof(cl_mem), (void *) & buf[3]);
	clSetKernelArg(D_kernel, 4, sizeof(cl_mem), (void *) & buf[3]);

	/* Запись в буферы buf[0], buf[1] и buf[2] исходных данных (массива реализаций случайной величины, массивов границ диапазонов) */
	clEnqueueWriteBuffer(com_queue, buf[0], CL_TRUE, 0, N * sizeof(float), X, 0, NULL, NULL);
	clEnqueueWriteBuffer(com_queue, buf[1], CL_TRUE, 0, n * sizeof(unsigned), from, 0, NULL, NULL);
	clEnqueueWriteBuffer(com_queue, buf[2], CL_TRUE, 0, n * sizeof(unsigned), to, 0, NULL, NULL);

	/* Выполнение ядра M st_n вычислительными потоками */
	clEnqueueNDRangeKernel(com_queue, M_kernel, 1, NULL, & st_n, NULL, 0, NULL, NULL);

	/* Чтение из буфера buf[3] результатов выполнения ядра M */
	clEnqueueReadBuffer(com_queue, buf[3], CL_TRUE, 0, n * sizeof(float), M, 0, NULL, NULL);

	/* Ожидание завершения выполнения всеми вычислительными потоками команд, помещенных в очередь команд функциями clEnqueue* */
	clFinish(com_queue);

	/* Расчет математического ожидания моделируемой случайной величины по результатам выполнения ядра M */
	for(u = 0, rM = 0; u < n; u++)
		rM += M[u];

	/* Определение шестого фактического параметра ядра D - математического ожидания моделируемой случайной величины */
	clSetKernelArg(D_kernel, 5, sizeof(unsigned), (void *) & rM);

	/* Выполнение ядра D st_n вычислительными потоками */
	clEnqueueNDRangeKernel(com_queue, D_kernel, 1, NULL, & st_n, NULL, 0, NULL, NULL);

	/* Чтение из буфера buf[3] результатов выполнения ядра D */
	clEnqueueReadBuffer(com_queue, buf[3], CL_TRUE, 0, n * sizeof(float), D, 0, NULL, NULL);

	/* Ожидание завершения выполнения всеми вычислительными потоками команд, помещенных в очередь команд функциями clEnqueue* */
	clFinish(com_queue);

	/* Расчет дисперсии моделируемой случайной величины по результатам выполнения ядра D */
	for(u = 0, rD = 0; u < n; u++)
		rD += D[u];

	/* Удаление всех использованных в данной программе экземпляров структур данных, описанных в стандарте OpenCL */

	for(u = 0; u < 4; u++)
		clReleaseMemObject(buf[u]);

	clReleaseKernel(M_kernel);
	clReleaseKernel(D_kernel);
	clReleaseProgram(prog);
	clReleaseCommandQueue(com_queue);
	clReleaseContext(context);

	/* ############################################################################ */

	/* Определение времени окончания расчетов */
	clock_gettime(CLOCK_REALTIME, & ts_after);

	/* Вывод результатов выполненных расчетов */
	printf("\nМатематическое ожидание:\n\n\tЭмпирическое = %f\n\tТеоретическое = %f\n\nДисперсия:\n\n\tЭмпирическая = %f\n\tТеоретическая = %f\n\nРасчет выполнили %d поток(а; ов) за %lf секунд(ы)\n\n", rM, exp(mu + sigma * sigma / 2), rD, exp(2 * mu + sigma * sigma) * (exp(sigma * sigma) - 1),
			n, ts_after.tv_sec - ts_before.tv_sec + 1 + (ts_after.tv_nsec - ts_before.tv_nsec) / 1000000000.0);

	free(from);
	free(to);
	free(X);
	free(M);
	free(D);
	free(src);

	return 0;
}

