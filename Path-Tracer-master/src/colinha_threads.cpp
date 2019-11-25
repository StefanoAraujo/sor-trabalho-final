#define N 4

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#define N_THREADS 4

long **res;
long **mat1;
long **mat2;

void *multiply(void *tid) 
{ 
	printf("thread %ld\n",(int) tid);
	int i, j, k;
	int t = (int) tid;
        //i = (int) tid;	
	int blocksize = N/N_THREADS;

  // AS THREADS ESTAO AQUI MAN
	for (i = (t)*blocksize; i < (t+1)*blocksize; i++) 
	{ 
		for (j = 0; j < N; j++) 
		{ 
			for (k = 0; k < N; k++) 
				res[i][j] += mat1[i][k] * mat2[k][j]; 
		} 
	}
  // *************************
	pthread_exit(NULL);
} 

int main(int argc, char **argv) 
{ 
	int i, j; 
	pthread_t threads[N_THREADS];
	int status;

	mat1 = (long *) malloc(N*sizeof(long));
	for(i = 0 ; i < N ; i ++)
		mat1[i] = (long*) malloc(N*sizeof(long));

	mat2 = (long *) malloc(N*sizeof(long));
	for(i = 0 ; i < N ; i ++)
		mat2[i] = (long*) malloc(N*sizeof(long));

  	res = (long *) malloc(N*sizeof(long));
	for(i = 0 ; i < N ; i ++)
		res[i] = (long*) malloc(N*sizeof(long));

	int count = 1;
	for (i = 0; i < N; i++) 
	{ 
		for (j = 0; j < N; j++) 
		{
			mat1[i][j] = count;
			mat2[i][j] = count;
			res[i][j] = 0;
			count++;
		}
	} 

	for(i = 0 ; i < N_THREADS ; i ++) 
	{
		status = pthread_create(&threads[i], NULL, multiply, (void*)i);
		if(status != 0)
		{
			printf("Cannot create thread\n");
			exit(1);
		}
	}

	for(i = 0 ; i < N_THREADS ; i ++) 
	{
		pthread_join(threads[i],NULL);
	}

	if(N < 5)
	{
		for (i = 0; i < N; i++) 
		{ 
			for (j = 0; j < N; j++) 
			{
				printf("%05ld ",res[i][j]);
			}
			printf("\n"); 
		} 
	}
	return 0; 
}
