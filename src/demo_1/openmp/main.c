
/*
 * Умножение матриц:
 *
 * A * B = R
 *
 * Здесь:
 *
 *	A - матрица N x M;
 *	B - матрица M x N;
 *	R - матрица N x N.
 */

#include "all.h"
#include "memory.h"

int main()
{
	int t_ind, t_num;
	unsigned u, v, t, N = 700, M = 5000;
	double t_R, ** A = NULL, ** B = NULL, ** R = NULL;
	struct timespec ts_before, ts_after;

	if(
		(A = (double **) alloc_2D(N, M, sizeof(double))) == NULL
		||
		(B = (double **) alloc_2D(M, N, sizeof(double))) == NULL
		||
		(R = (double **) alloc_2D(N, N, sizeof(double))) == NULL
	  )
	{
		free_ND(A);
		free_ND(B);
		free_ND(R);

		ERROR;
	}

	srand48(lrand48());

	for(v = 0; v < N; v++)
		for(u = 0; u < M; u++)
		{
			A[v][u] = mrand48() % 10 + drand48();
			B[u][v] = mrand48() % 10 + drand48();
		}

	// ############################################################################ 

	omp_set_num_threads(1);
	t_num = omp_get_max_threads();

	clock_gettime(CLOCK_REALTIME, & ts_before);

	#pragma omp parallel private(v, u, t, t_R)
	{
		t_ind = omp_get_thread_num();

		for(v = t_ind; v < N; v += t_num)
			for(u = 0; u < N; u++)
			{
				for(t = 0, t_R = 0; t < M; t++)
					t_R += A[v][t] * B[t][u];

				R[v][u] = t_R;
			}
	}

	clock_gettime(CLOCK_REALTIME, & ts_after);

	fprintf(stderr, "%d поток(а; ов) = %lf секунд(ы)\n",
			t_num, ts_after.tv_sec - ts_before.tv_sec + 1 + (ts_after.tv_nsec - ts_before.tv_nsec) / 1000000000.0);

	// ############################################################################ 

	dprintf(3, "#!/usr/bin/octave --silent\n\nA = [ ");

	for(v = 0; v < N; v++)
	{
		for(u = 0; u < M - 1; u++)
			dprintf(3, "%lf, ", A[v][u]);

		dprintf(3, "%lf ", A[v][u]);

		if(v < N - 1)
			dprintf(3, "; ");
	}

	dprintf(3, "];\nB = [ ");

	for(v = 0; v < M; v++)
	{
		for(u = 0; u < N - 1; u++)
			dprintf(3, "%lf, ", B[v][u]);

		dprintf(3, "%lf ", B[v][u]);

		if(v < M - 1)
			dprintf(3, "; ");
	}

	dprintf(3, "];\nR = [ ");

	for(v = 0; v < N; v++)
	{
		for(u = 0; u < N - 1; u++)
			dprintf(3, "%lf, ", R[v][u]);

		dprintf(3, "%lf ", R[v][u]);

		if(v < N - 1)
			dprintf(3, "; ");
	}

	dprintf(3, "];\nmax(max(A * B - R))\n\n");

	free_ND(A);
	free_ND(B);
	free_ND(R);

	return 0;
}

