/**
 * Defines some operation for bbpeer, mainly file operation
 *
 * @author : Probal chandra dhar
 * @Filename : peer.h
 * @Date : 03/17/17
 * @course : COP5990
 * @Project # : 3
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<unistd.h>
#include 	<netdb.h>
#include 	<pthread.h>
#include 	<semaphore.h>

#ifndef _PEER_H
#define _PEER_H

// Message length
#define MESSAGE 54

#define BUFSIZE 512
#define MAXSERVERNAME 100

sem_t semaphore;

// Structure to pass more than one argument through thread argument
typedef struct threadArgs
{	
	// destination IP & port of a peer
	struct sockaddr_in *dest;
	// socket descriptor of a peer
	int socketID;
	// FILE pointer
	FILE *filefp;

}threadArgs;


/**
 * Thread function
 *
 * @param param - structure of data needed
 *
 */
void* threadFunc ( void* param );


/**
 * connection establishment function
 *
 * @param 	fp - file pointer, can be stdin
 * @param 	sockfd - socket descriptor
 * @param 	to - socket address structure, containing IP address and port number of the server
 * @param 	length - length of the socket address structure
 *
 * @return 	0 if successful -1 otherwise.
 *
 */
int main_functionlity(FILE *, int, struct sockaddr * ,socklen_t);

/**
 * function to write the file
 *
 * @param 	message - message to write the file
 * @param 	index - index where the file should write
 * @param 	fp - File pointer
 *
 * @return 	0 if successful, -1 if failed
 *
 */
int writeFile (char *message, unsigned int index, FILE* fp);


/**
 * function to read the file
 *
 * @param 	message - message to write the file
 * @param 	index - index where the file should write
 * @param 	fp - File pointer
 *
 * @return 	0 if successful, -1 if failed
 *
 */
int readFile (char *message, unsigned int index, FILE* fp);

/**
 * function to write the file & display confirmation message
 *
 * @param 	fp - File pointer
 *
 * @return 	0 if successful, -1 otherwise
 *
 */
int doFileWrite(FILE *fp);

/**
 * function to read the file & display the message
 *
 * @param 	fp - File pointer
 *
 * @return 	0 if successful, -1 otherwise
 *
 */
int doFileRead( FILE *fp );

/**
 * function to calculate the file size
 *
 * @param 	fp - File pointer
 *
 * @return 	index if successful, -1 otherwise
 *
 */
int calcFileSize( FILE *fp );







#endif




