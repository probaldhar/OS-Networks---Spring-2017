/**
 * Implements all operation for a mt-collatz.
 *
 * @author : Probal chandra dhar
 * @Filename : mt-collatz.c
 * @Date : 02/11/17
 * @course : COP5990
 * @Project # : 2
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "mt-collatz.h"
#include "time.h"

// global array to store the stopping time
int globalarray[500];

// setting the global COUNTER to 2
int COUNTER = 2;

// thread argument struct
threadArgs args;

// thread mutex variable declare 
pthread_mutex_t lock, lock2;

/**
 * Thread function
 *
 * @param 	number - number to find out the collatz sequence for
 *
 * @return 	stepsNeeded - stopping time to calulate collatz sequence
 *
 */
int calculateCollatzSequence( unsigned long int number ) {

	// initializing the steps
	unsigned long int stepsNeeded = 1;

	while ( number != 1 ) {

		if ( number % 2 == 0 ) {
			// number is even
			number = number / 2;
		} else {
			// number os odd
			number = ( 3 * number ) + 1;
		}

		stepsNeeded++;
	}

	return stepsNeeded;

}


/**
 * Thread function
 *
 * @param param - input all the argument structure
 *
 */
void* threadFunc ( void* param )
{
	// storing the present counter to nextNum
	unsigned long int nextNum;

	// threadArgs args = *(threadArgs *) param;
	// int threadId = *(int *)param;

	// storing the range to N form the thread argument
	int N = args.Nrange;

	// stopping time to calulate collatz sequence
	int stepsNeeded;

	while ( 1 ) {

		// using mutex only if there's no -nolock
		if ( args.noLock == 0 ) {

			// initiating the lock 
			pthread_mutex_lock(&lock);

				// increasing the value of COUNTER
				nextNum = COUNTER++;

				// breaking the loop
				if (COUNTER >= N) {
					pthread_mutex_unlock(&lock);
					break;
				}

			// mutex unlock
			pthread_mutex_unlock(&lock);
		} else {

			// No lock 

			// increasing the value of COUNTER
			nextNum = COUNTER++;

			// breaking the loop
			if ( COUNTER >= N ) 
				break;

		}

		// calculating the collatz sequence
		stepsNeeded = calculateCollatzSequence (nextNum);

		// using mutex only if there's no -nolock
		if ( args.noLock == 0 ) { 

			// initiating the lock 
			pthread_mutex_lock(&lock2);

				// updating the global histogram array
				globalarray[stepsNeeded]++;

			// mutex unlock
			pthread_mutex_unlock(&lock2);

		} else {

			// updating the global histogram array
			globalarray[stepsNeeded]++;

		}

	}

	// exiting the main thread
	pthread_exit(0);
}


int main(int argc, char **argv){

	// starts the clock to calculate the elapsed time
	startTimer();

	// the range of numbers for which a Collatz sequence must be computed
	int N;

	// loop counter
	int i = 0;

	// thread ID array
	pthread_t tid[30];

	// Initially assuming there's no -nolock given in the commandline argument
	args.noLock = 0;

	// Usage
	if ( argc < 3 ) {
		printf("Usage: ./mt-collatz Range Threads [-nolock]\n");
		exit(1);
	} else if ( argc >= 4 && strcmp( argv[3], "-nolock" )) {
		printf("Usage: ./mt-collatz Range Threads [-nolock]\n");
		exit(1);
	} else if ( ( atoi( argv[1] ) == 0 ) || ( atoi( argv[2] ) == 0 ) ) {
		printf("Range of numbers and number of threads must be a number.\n");
		exit(1);
	} else if ( argc >= 4 && !strcmp( argv[3], "-nolock" ) ) {
		args.noLock = 1;
	}


	// the number of threads to create to compute the results in parallel
	int T = atoi(argv[2]);
    
	// setting the value of N from the first argument
	N = atoi(argv[1]);

	// assigning the N to the thread argument
	args.Nrange = N;

	// Checking for -nolock commandline argument
	if ( args.noLock == 0 ) { 

		if ( pthread_mutex_init(&lock, NULL) != 0 ) {
	        printf("\n mutex init failed\n");
	   		return 1;
	   	}

		pthread_mutex_init(&lock2, NULL);

	}

	// setting 0 to all values in the global array
	for ( i = 0; i < 500; i++ ) {
		globalarray[i] = 0;
	}

	// checking for error
	int err;

	// passing the thread ID to the thread function
	int threadIds[T];

	// creating threads
	for ( i = 0; i < T; i++ ) {
		
		// storing the thread ID to the threadIds array
		threadIds[i] = i;

		err = pthread_create(&tid[i], NULL, threadFunc, (void *)&threadIds[i] );
        	
		if (err != 0) {
   			printf("\ncan't create thread :[%s]", strerror(err));
		}

    }

    // joining the thread to the main thread
   	for ( i = 0; i < T; i++)
	{
		pthread_join(tid[i], NULL);
	}

	// Check for -nolock argument in the commandline argument
	if ( args.noLock == 0 ) { 

		// destroy the mutex variables
		pthread_mutex_destroy(&lock);

		pthread_mutex_destroy(&lock2);
	}

	// printing the histogram array
	for ( i = 0; i < 500; i++ ) {
		printf("%d, %d\n", (i + 1), globalarray[i]);
	}

	// stop the clock that starts in the start of the program
	float elapsed = stopTimer();

    // prints the elapsed time
	fprintf (stderr, "%d,%d,%f\n", N, T, elapsed);

	// make the compiler happy
	return 0;

}
