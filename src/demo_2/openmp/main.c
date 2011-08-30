
/*

Многопоточное моделирование распределения Стьюдента с помощью датчика псевдослучайных чисел, распределенных равномерно

Сборка программы:

	gcc -Wall -fopenmp -lm -lrt main.c -o demo

	Здесь:

		-Wall - ключ, предписывающий компилятору выводить предупреждения о всех потенциальных ошибках в программе;
		-fopenmp - ключ, подключающий к программе функционал стандарта OpenMP, реализованный используемым компилятором;
		-lm - ключ, подключающий к программе математическую библиотеку из состава стандартной библиотеки языка программирования C;
		-lrt - ключ, подключающий к программе библиотеку получения информации о времени;
		main.c - файл с исходным кодом программы;
		-o demo - указание имени результирующего исполняемого файла.

Запуск программы:

	./demo TNUM FL N

	Здесь:

		TNUM - количество вычислительных потоков;
		FL - число степеней свободы распределения Стьюдента (должно быть больше двух);
		N - количество вычисляемых реализаций псевдослучайной величины, распределенной по Стьюденту.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <omp.h>

/* Псевдослучайные числа, генерируемые функцией drand48(), равномерно распределены на диапазоне [0; 1) - функция normal_distrib() генерирует псевдослучайные числа, нормально распределенные на диапазоне (-1; 1) */
double normal_distrib()
{

#define X 12
#define X_2 6

	unsigned k;
	double epsilon;

	/* Центральная предельная теорема */

	for(k = 0, epsilon = 0; k < X; k++)
		epsilon += drand48();

	return (epsilon - X_2) / X_2;
}

/* Главная функция программы */
int main(const int argc, const char * argv[])
{
	if(argc != 4)
	{
		fprintf(stderr, "\nНедостаточно или избыток аргументов\n\n");
		return -1;
	}

	unsigned u, v, t_ind, t_num, n = atoi(argv[2]), N = atoi(argv[3]);

	if(n < 3)
	{
		fprintf(stderr, "\nЧисло степеней свободы распределения Стьюдента должно быть больше двух)\n\n");
		return -1;
	}

	double tx, su, m = 0, d = 0, * x = calloc(N, sizeof(double));
	struct timespec ts_before, ts_after;

	/* Инициализация датчика псевдослучайных чисел */
	srand48(lrand48());

	/* Количество вычислительных потоков устанавливается в значение первого аргумента программы */
	omp_set_num_threads(atoi(argv[1]));

	/* Получение количества вычислительных потоков */
	t_num = omp_get_max_threads();

	/* Определение времени начала расчетов */
	clock_gettime(CLOCK_REALTIME, & ts_before);

	/*
	
	Начало параллельной секции кода.

	Переменные t_ind, u, v, tx, su будут составлять локальные контексты вычислительных потоков, все прочие переменные будут разделяемыми.

	*/
	#pragma omp parallel private(t_ind, u, v, tx, su)
	{
		/* Получение номера потока */
		t_ind = omp_get_thread_num();

		/* Вычисление эмпирического математического ожидания */
		for(u = t_ind; u < N; u += t_num)
		{
			/* Генерирование очередной реализации псевдослучайной величины, распределенной по Стьюденту */
			for(v = 0, tx = 0; v < n; v++)
			{
				su = normal_distrib();
				tx += su * su;
			}

			x[u] = tx = normal_distrib() / sqrt(tx / n);

			/* В критической секции кода один и только один вычислительный поток в определенный момент времени увеличивает значение аккумулятора */
			#pragma omp critical
			{
				m += tx;
			}
		}

		/* Синхронизация потоков */
		#pragma omp barrier

		/* Главный вычислительный поток вычисляет эмпирическое математическое ожидание */
		#pragma omp single
		{
			m /= (double) N;
		}

		/* Синхронизация потоков */
		#pragma omp barrier

		/* Вычисление эмпирической дисперсии */
		for(u = t_ind; u < N; u += t_num)
		{
			tx = x[u] - m;

			/* В критической секции кода один и только один вычислительный поток в определенный момент времени увеличивает значение аккумулятора */
			#pragma omp critical
			{
				d += tx * tx;
			}
		}
	}

	/* Главный вычислительный поток вычисляет эмпирическую дисперсию */
	d /= (double) N;

	/* Определение времени окончания расчетов */
	clock_gettime(CLOCK_REALTIME, & ts_after);

	/* Вывод результатов выполненных расчетов */
	printf("\nМатематическое ожидание:\n\n\tЭмпирическое = %lf\n\tТеоретическое = 0\n\nДисперсия:\n\n\tЭмпирическая = %lf\n\tТеоретическая = %lf\n\nРасчет выполнили %d поток(а; ов) за %lf секунд(ы)\n\n", m, d, n / (double) (n - 2), t_num, ts_after.tv_sec - ts_before.tv_sec + 1 + (ts_after.tv_nsec - ts_before.tv_nsec) / 1000000000.0);

	free(x);

	return 0;
}

