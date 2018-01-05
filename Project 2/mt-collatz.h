/**
 * Defines all operation for mt-collatz
 *
 * @author : Probal chandra dhar
 * @Filename : mt-collatz.h
 * @Date : 02/11/17
 * @course : COP5990
 * @Project # : 2
 */

#include <pthread.h>

#ifndef _MT_COLLATZ_H
#define _MT_COLLATZ_H

// Structure to pass more than one argument through thread argument
typedef struct threadArgs
{
	int Nrange;
	int noLock;
}threadArgs;

/**
 * Thread function
 *
 * @param param - input all the argument structure
 *
 */
void* threadFunc ( void* param );

/**
 * Thread function
 *
 * @param 	number - number to find out the collatz sequence for
 *
 * @return 	stepsNeeded - stopping time to calulate collatz sequence
 *
 */
int calculateCollatzSequence( unsigned long int number );













#endif


