#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
typedef float real_t;
typedef real_t ***arr_t;
#define MAX_TIME 2
#define INNER -1.0
#define OUTER 1.0

inline
void setElement(float ***array, float value, int x, int y, int z)
{
	// 在一个三维数组里面，指定位置里面设置位置
	array[x][y][z] = value;
}

void initValues(float ***array, int sx, int sy, int sz, float inner_temp, float outer_temp)
{
	#pragma omp parallel for shared(array, sx, sy, sz, inner_temp)
	for (int i = 1; i < (sx - 1); i++)
	{
		for (int j = 1; j < (sy - 1); j++)
		{
			for (int k = 1; k < (sz - 1); k++)
			{
				setElement(array, inner_temp, i, j, k);
			}
		}
	}

	#pragma omp parallel for shared(array, sx, sy, sz, outer_temp)
	for (int j = 0; j < sy; j++)
	{
		for (int k = 0; k < sz; k++)
		{
			setElement(array, outer_temp, 0, j, k);
			setElement(array, outer_temp, sx - 1, j, k);
		}
	}

	#pragma omp parallel for shared(array, sx, sy, sz, outer_temp)
	for (int i = 0; i < sx; i++)
	{
		for (int k = 0; k < sz; k++)
		{
			setElement(array, outer_temp, i, 0, k);
			setElement(array, outer_temp, i, sy - 1, k);
		}
	}
	
	#pragma omp parallel for shared(array, sx, sy, sz, outer_temp)
	for (int i = 0; i < sx; i++)
	{
		for (int j = 0; j < sy; j++)
		{
			setElement(array, outer_temp, i, j, 0);
			setElement(array, outer_temp, i, j, sz - 1);
		}
	}
}

void stencil_3d_7point(arr_t A, arr_t B, const int nx, const int ny, const int nz)
{

	int i, j, k;

	for (int timestep = 0; timestep < MAX_TIME; ++timestep)
	{	
		#pragma omp parallel for
		for (i = 1; i < nx - 1; i++)
			for (j = 1; j < ny - 1; j++)
				for (k = 1; k < nz - 1; k++)
					B[i][j][k] = (A[i - 1][j][k] +
								  A[i][j - 1][k] +
								  A[i][j][k - 1] +
								  A[i][j][k] +
								  A[i][j][k + 1] +
								  A[i][j + 1][k] +
								  A[i + 1][j][k]) /
								 7.0;
		#pragma omp parallel for
		for (i = 1; i < nx - 1; i++)
			for (j = 1; j < ny - 1; j++)
				for (k = 1; k < nz - 1; k++)
					A[i][j][k] = B[i][j][k];
	}
}

int main()
{
	printf("Running parallel stencil, threads = 4......................................\n");
	double start = omp_get_wtime();
	int i, j, k;
	int sz = 1500;
	int sx = 1500;
	int sy = 1500;
	// 为三维数组A分配内存空间
	float ***A = (float ***)malloc(sx * sizeof(float **));

	double ts = omp_get_wtime();
	#pragma omp parallel for shared(A)
	for (i = 0; i < sy; i++)
	{
		A[i] = (float **)malloc(sy * sizeof(float *));
		for (j = 0; j < sz; j++)
		{
			A[i][j] = (float *)malloc(sz * sizeof(float));
		}
	}
	// 为三维数组A分配内存空间
	float ***B = (float ***)malloc(sx * sizeof(float **));
	#pragma omp parallel for shared(B)
	for (i = 0; i < sy; i++)
	{
		B[i] = (float **)malloc(sy * sizeof(float *));
		for (j = 0; j < sz; j++)
		{
			B[i][j] = (float *)malloc(sz * sizeof(float));
		}
	}
	double tf = omp_get_wtime();
	
	printf("Time allocating: %.4lfs\n", tf - ts);
  	
	// initialize the A matrix
	ts = omp_get_wtime();

	initValues(A, sx, sy, sz, INNER, OUTER);
	
	tf = omp_get_wtime();
	printf("Time initiating: %.4lfs\n", tf - ts);

	
	
  	ts = omp_get_wtime();

	stencil_3d_7point(A, B, sz, sy, sz);

	tf = omp_get_wtime();
	
  	printf("Time stencil: %.4lfs\n", tf - ts);
	free(A);
	double end = omp_get_wtime();
	printf("Overall Time : %.4lfs\n", end - start);

	// delete[] A;
	return 0;
}
