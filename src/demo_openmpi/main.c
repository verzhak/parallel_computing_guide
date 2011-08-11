
#include "all.h"
#include "memory.h"

int main(int argc, char * argv[])
{
	int ret;
	unsigned size, size_1;
	double coef, sum, sko, delta, ** A = NULL, ** tA = NULL, * b = NULL, * tb = NULL, * x = NULL;
	int t_num, t_ind, t_t, from_v, to_v, num, eq, eq_per_thread, last_eq_per_thread, u, v, t, last;
	struct timespec ts_before, ts_after;
	MPI_Status status;

	if((ret = MPI_Init(& argc, & argv)) != MPI_SUCCESS)
		ERROR;

	MPI_Comm_size(MPI_COMM_WORLD, & t_num);
	MPI_Comm_rank(MPI_COMM_WORLD, & t_ind);

	if(! t_ind)
		size = 10;

	MPI_Bcast(& size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	size_1 = size - 1;

	if(
		(A = (double **) alloc_2D(size, size, sizeof(double))) == NULL
		||
		(tA = (double **) alloc_2D(size, size, sizeof(double))) == NULL
		||
		(b = alloc_1D(size, sizeof(double))) == NULL
		||
		(tb = alloc_1D(size, sizeof(double))) == NULL
		||
		(x = alloc_1D(size, sizeof(double))) == NULL
	  )
	{
		free_ND(A);
		free_ND(tA);
		free_ND(b);
		free_ND(tb);
		free_ND(x);

		ERROR;
	}

	if(! t_ind)
	{
		srand48(lrand48());

		for(u = 0; u < size; u++)
		{
			for(v = 0; v < size; v++)
				A[u][v] = mrand48() % 10000 + drand48();

			b[u] = mrand48() % 10000 + drand48();
		}
		
		memcpy((void *) (tA + size), (void *) (A + size), size * size * sizeof(double));
		memcpy(tb, b, size * sizeof(double));

		clock_gettime(CLOCK_REALTIME, & ts_before);
	}

	for(u = 0; u < size_1; u++)
	{
		MPI_Bcast(tA[u], size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(tb + u, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		if(! t_ind)
		{
			printf("------------------> %u\n", u);

			// Главный поток

			if(! (eq_per_thread = (size - u - 1) / (t_num - 1)))
			{
				eq_per_thread = 1;
				last = size - u;
			}
			else
				last = t_num;

			for(t = 0; t < t_num; t ++)
				MPI_Send(& last, 1, MPI_INT, t, 7, MPI_COMM_WORLD);

			for(t_t = 1, eq = u + 1; t_t < (t_num - 1) && eq < size; t_t ++, eq += eq_per_thread)
			{
				MPI_Send(& eq, 1, MPI_INT, t_t, 1, MPI_COMM_WORLD);
				MPI_Send(& eq_per_thread, 1, MPI_INT, t_t, 2, MPI_COMM_WORLD);
				MPI_Send(tA[eq], size * eq_per_thread, MPI_DOUBLE, t_t, 5, MPI_COMM_WORLD);
				MPI_Send(tb + eq, eq_per_thread, MPI_DOUBLE, t_t, 6, MPI_COMM_WORLD);
			}

			if(eq < size)
			{
				MPI_Send(& eq, 1, MPI_INT, t_t, 1, MPI_COMM_WORLD);

				last_eq_per_thread = size - eq;

				MPI_Send(& last_eq_per_thread, 1, MPI_INT, t_t, 2, MPI_COMM_WORLD);
				MPI_Send(tA[eq], size * last_eq_per_thread, MPI_DOUBLE, t_t, 5, MPI_COMM_WORLD);
				MPI_Send(tb + eq, last_eq_per_thread, MPI_DOUBLE, t_t, 6, MPI_COMM_WORLD);

				t_t ++;
			}

			for(t_t = 1, eq = u + 1; t_t < (t_num - 1) && eq < size; t_t ++, eq += eq_per_thread)
			{
				MPI_Recv(tA[eq], size * eq_per_thread, MPI_DOUBLE, t_t, 3, MPI_COMM_WORLD, & status);
				MPI_Recv(tb + eq, eq_per_thread, MPI_DOUBLE, t_t, 4, MPI_COMM_WORLD, & status);
			}

			if(t_t == t_num - 1 && eq < size)
			{
				MPI_Recv(tA[eq], size * (size - eq), MPI_DOUBLE, t_t, 3, MPI_COMM_WORLD, & status);
				MPI_Recv(tb + eq, size - eq, MPI_DOUBLE, t_t, 4, MPI_COMM_WORLD, & status);
			}
		}
		else
		{
			// Вычислительные потоки

			MPI_Recv(& last, 1, MPI_INT, 0, 7, MPI_COMM_WORLD, & status);

			if(t_ind >= last)
				break;

			MPI_Recv(& from_v, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, & status);
			MPI_Recv(& num, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, & status);
			MPI_Recv(tA[from_v], size * num, MPI_DOUBLE, 0, 5, MPI_COMM_WORLD, & status);
			MPI_Recv(tb + from_v, num, MPI_DOUBLE, 0, 6, MPI_COMM_WORLD, & status);

			to_v = from_v + num;

			for(v = from_v; v < to_v; v++)
			{
				coef = tA[v][u] / tA[u][u];

				for(t = 0; t < size; t++)
					tA[v][t] -= coef * tA[u][t];

				tb[v] -= coef * tb[u];
			}

			MPI_Send(tA[from_v], size * num, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
			MPI_Send(tb + from_v, num, MPI_DOUBLE, 0, 4, MPI_COMM_WORLD);
		}
	}

	if(! t_ind)
	{
		for(u = size_1; u >= 0; u--)
		{
			for(v = size_1, sum = 0; v > u; v--)
				sum += x[v] * tA[u][v];

			x[u] = (tb[u] - sum) / tA[u][u];
		}

		clock_gettime(CLOCK_REALTIME, & ts_after);

		for(u = 0, sko = 0; u < size; u++)
		{
			for(v = 0, sum = 0; v < size; v++)
				sum += x[v] * A[u][v];

			delta = sum - b[u];
			sko += delta * delta;
		}

		fprintf(stderr, "СКО = %.11lf\n%d поток(а; ов) = %.7lf секунд(ы)\n",
			sqrt(sko / size), t_num, ts_after.tv_sec - ts_before.tv_sec + 1 + (ts_after.tv_nsec - ts_before.tv_nsec) / 1000000000.0);
	}

	free_ND(A);
	free_ND(tA);
	free_ND(b);
	free_ND(tb);
	free_ND(x);

	MPI_Finalize();

	return 0;
}

