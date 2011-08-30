
/*

Исходный код ядер (главных функций) вычислительных потоков

Для программирования ядер (главных функций) вычислительных потоков используется диалект языка программирования C, обладающий следующими особенностями:

	1. Стандартная библиотека не доступна, доступен только функционал, предусмотренный стандартом OpenCL;
	2. С помощью ключевого слова __kernel необходимо отметить ядра (главные функции) вычислительных потоков; с помощью ключевого слова __global необходимо отметить переменные, расположенные в глобальной памяти;
	3. Тип данных double реализуется в расширении к стандарту OpenCL и поддерживается не всеми видеокартами - использование типа данных double нецелесообразно из-за потенциальной необходимости переписывать исходный код ядер (главных функций) вычислительных потоков при переносе программы на другую аппаратную платформу.

*/

/* Ядро (главная функция) вычислительных потоков, рассчитывающих сумму элементов в соответствующем подмассиве массива реализаций псевдослучайной величины */
__kernel void M(__global const float * X, unsigned N, __global const unsigned * from, __global const unsigned * to, __global float * res)
{
	size_t t_ind = get_global_id(0);
	unsigned u, t_to = to[t_ind];
	float t_res;

	for(u = from[t_ind], t_res = 0; u < t_to; u++)
		t_res += X[u];

	res[t_ind] = t_res / N;
}

/* Ядро (главная функция) вычислительных потоков, рассчитывающих сумму квадратов центрированных реализаций псевдослучайной величины в соответствующем подмассиве массива реализаций псевдослучайной величины */
__kernel void D(__global const float * X, unsigned N, __global const unsigned * from, __global const unsigned * to, __global float * res,
		float M)
{
	size_t t_ind = get_global_id(0);
	unsigned u, t_to = to[t_ind];
	float d, t_res;

	for(u = from[t_ind], t_res = 0; u < t_to; u++)
	{
		d = (X[u] - M);
		t_res += d * d;
	}

	res[t_ind] = t_res / N;
}
