/**
 * Implements all operation for client.
 *
 * @author : Probal
 * @Filename : server.c
 * @Date : 04/17/17
 * @course : COP5990
 * @Project # : 5
 * @Usage: ./node <routerLabel> <portNum> <totalNumRouters> <discoverFile> [-dynamic]
 */

#include	<stdio.h>
#include	<stdlib.h>
#include 	<time.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/wait.h>
#include 	<netdb.h>
/* According to POSIX.1-2001, POSIX.1-2008 */
#include 	<sys/select.h>
/* According to earlier standards */
#include 	<sys/time.h>
#include 	<sys/types.h>
#include 	<unistd.h>

#include	"node.h"

// global adjacency matrix
int **adjMat;

int main(int argc,  char *argv[] ) {

	// For return values for socket creation, send & receive message
	int nodeSd, sendRet, dynamic = 0, i, k, debug = 0;
	// sockaddr_in to send the message
	struct sockaddr_in ownAddress, serveraddress, recvAddress, neighbors[NUM_NEIGHBOR]; // neighbors to make the connections
	// length of sockaddr_in
	socklen_t addrlen = sizeof(serveraddress);
	// get the host information
	struct hostent *hostptr;
	// store the hostname
	char hostname[128];
	// For storing the argument
    char *portNum, *rounterLabel, *totalNumRouters, *discoverFile, FileNameWithPath[BUFSIZE];

	// Link state packets for each record in the file
	allLSP allLSP, dynamicLSP;

	// Usage
	if ( argc < 5 ){
		printf("Usage: ./node <routerLabel> <portNum> <totalNumRouters> <discoverFile> [-dynamic] [-Debug]\n");
		exit(1);
	}

    // setting memory for the argument strings separately
	rounterLabel = (char * )malloc(sizeof(char)*BUFSIZE);
	portNum = (char * )malloc(sizeof(char)*BUFSIZE);
	totalNumRouters = (char * )malloc(sizeof(char)*BUFSIZE);
	discoverFile = (char * )malloc(sizeof(char)*BUFSIZE);

	// check if dynamic
	if ( argc == 6 && !strcmp(argv[5], "-dynamic") )
		dynamic = 1; // setting the flag to 1
	else if ( argc == 7 && !strcmp(argv[5], "-dynamic") )
		dynamic = 1; 

	// check if debug
	if ( argc == 6 && !strcmp(argv[5], "-Debug") )
		debug = 1; // setting the flag to 1
	else if ( argc == 7 && !strcmp(argv[6], "-Debug") )
		debug = 1; // setting the flag to 1

	/****** storing the arguments *****/
	rounterLabel = argv[1]; // router label
	portNum = argv[2]; // own port
	totalNumRouters = argv[3]; // Total number of routers
	discoverFile = argv[4]; // File for neighbor discovery

	//converting totalNumRouters - string to int
	int rowCol = atoi(totalNumRouters);

	// sequence matrix
	int seqMat[rowCol][rowCol];

	// dynamic memory allocation for adjacency matrix
	adjMat = (int **)malloc(rowCol * sizeof(int *));
	for ( i = 0; i < rowCol; i++ )
		 adjMat[i] = (int *)malloc(rowCol * sizeof(int));

	// assign -1 to each of the record of adjMat
	for ( i = 0; i < rowCol; i++ ) {
		for ( k = 0; k < rowCol; k++ ) {

			if ( i == k ) {
				// adjacency matrix
				adjMat[i][k] = 0;
				// sequence matrix
				seqMat[i][k] = 0;
			} else {
				// adjacency matrix
				adjMat[i][k] = inf;
				// sequence matrix
				seqMat[i][k] = inf;
			}
		}
	}

	if ( debug ) {
		printf("matrix check\n");
		printArray(rowCol, adjMat);
	}

	// combining the filename with path
	if ( snprintf(FileNameWithPath, sizeof(FileNameWithPath), "%s%s", "routers/", discoverFile) < 0 ) {
		printf("something went wrong with snprintf:");
		exit(1);
	}	

	// creating socket
	if ( (nodeSd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("socket creation failed");
		return -1;
	}

	// // Get information about client:
	if ( gethostname(hostname, sizeof(hostname) ) ){
		printf("Error\n");
	}

	// get network host entry
    hostptr = gethostbyname(hostname);
    // hostptr = gethostbyname("127.0.0.1"); // THIS SHOULD BE REPLACED BY THE PREVIOUS LINE

	// setting memory to recvAddress 
	memset( &recvAddress, 0, sizeof(recvAddress) );

	// own address
	memset( &ownAddress, 0, sizeof(ownAddress) );
	ownAddress.sin_family = AF_INET;
	ownAddress.sin_port = htons(atoi(portNum));//PORT NO
	ownAddress.sin_addr.s_addr = htonl(INADDR_ANY);//ADDRESS

	memcpy((void *)&ownAddress.sin_addr, (void *)hostptr->h_addr, hostptr->h_length); 

	if ( debug ) {
		// printing the hostname, IP & port
		printf("name: %s\n", hostptr->h_name);
	    printf("addr: [%s]\n", inet_ntoa(ownAddress.sin_addr));
	    printf("port: %d\n",ntohs(ownAddress.sin_port));
		
		printf("router Starting service\n");
	}

	// binding to a specific port
	if ( bind(nodeSd, (struct sockaddr *) &ownAddress, sizeof(ownAddress) ) < 0 ){
		perror("Bind Fails:");
		exit(1);
	}

	// File operation
	FILE *fp = fopen(FileNameWithPath, "r");
	// delimeter
	const char s[2] = ", ";
    char *token;

	// neighbor counter
	int neighborCounter = 0;

	// Check if the file is open or not
	if(fp == NULL)
    {
        perror("cannot open File:");
		exit(1);
    }

	// Tokenize a line from the file
	char line[32];
	while(fgets(line, sizeof(line), fp) != NULL)
	{

		// loop counter
		i = 0;

		/********* LSP create for each record in the file *********/

		// copying source router's label to allLSP
		// sequence number, initially 1
		allLSP.singleLSP[neighborCounter].seqNum = 1;

		token = strtok(line, s);

		while (token!= NULL)
		{
			// routerLabel
			if ( i % 4 == 0 ) { // rounterLabel
				strcpy(allLSP.singleLSP[neighborCounter].label, token);
			} else if ( i % 4 == 1 ) { // IP_address/hostname

				strcpy(allLSP.singleLSP[neighborCounter].nodeIP, token);
			} else if ( i % 4 == 2 ) { // portNumber
				allLSP.singleLSP[neighborCounter].nodePort = atoi(token);
			} else {
				allLSP.singleLSP[neighborCounter].cost = atoi(token);
			}

			token = strtok (NULL, s);
			i++; // loop counter
		}

		neighborCounter++; // adding the neighbor counter

	}


	// initial sleep
	printf("initial %d seconds delay so that every router can up & running.\n", TIMEOUT);
	sleep(TIMEOUT);

	// assign the number of neighbor in the LSP packet
	allLSP.numberOfNeighbor = neighborCounter;

	// assigning hop count to packet
	allLSP.hopCount = atoi(totalNumRouters) - 1;

	// copy the router source label to the packet
	strcpy(allLSP.source, rounterLabel);

	// packet from FILE
	int j;
	for ( j = 0; j < neighborCounter; j++ ) {

		// initial matrix build
		adjMatrixChange( adjMat, allLSP.source, allLSP.singleLSP[j].label, allLSP.singleLSP[j].cost );

		seqMatrixChange( rowCol, seqMat, allLSP.source, allLSP.singleLSP[j].label, allLSP.singleLSP[j].seqNum );

		if ( debug ) {
			// printing the values of LSPs
			printf("%s %d %s %s %d %d\n", allLSP.source, allLSP.singleLSP[j].seqNum, allLSP.singleLSP[j].label, allLSP.singleLSP[j].nodeIP, allLSP.singleLSP[j].nodePort, allLSP.singleLSP[j].cost);
		}

		// what if the file don't have IP, have to find out IP from hostname
		hostptr = gethostbyname(allLSP.singleLSP[j].nodeIP);

		// error checking
	    if ( hostptr == NULL ){
	    	perror("NULL when calculating gethostbyname from file hostname:");
	    	return 0;
	    }

		// sockaddr_in create for each record in the file
		memset( &neighbors[j], 0, sizeof(neighbors[j]) );
		neighbors[j].sin_family = AF_INET;
		neighbors[j].sin_port = htons(allLSP.singleLSP[j].nodePort);//PORT NO
		neighbors[j].sin_addr.s_addr = inet_addr(allLSP.singleLSP[j].nodeIP);//ADDRESS

		memcpy((void *)&neighbors[j].sin_addr, (void *)hostptr->h_addr, hostptr->h_length); 

		if ( debug ) {
			printf("Neighboring connection information.\n");
			printf("name: %s\n", hostptr->h_name);
	    	printf("addr: [%s]\n", inet_ntoa(neighbors[j].sin_addr));
	    	printf("port: %d\n",ntohs(neighbors[j].sin_port));
	    }
		// size of sockaddr_in
		addrlen = sizeof(neighbors[i]);

		// send "allLSP" to every record of the neighbors
		sendRet = sendto( nodeSd, &allLSP, sizeof(allLSP), 0, (struct sockaddr*)&neighbors[j],addrlen);

		if ( sendRet < 0 ) {
			perror("something went wrong while sending:");
			exit(1);
		}

	}

	if ( debug ) {
		// printing adjacency matrix so far
		printArray(rowCol, adjMat);
	}

	// receive & send packet
	floodReceiveWithSelect( nodeSd, neighbors, rowCol, adjMat, neighborCounter, seqMat, debug );

	/********* start after receive *********

		1. check the hop count 										- DONE
		2. check where it's coming from								- DONE
		3. send the packet if hop > 0 to others router 				- DONE
		4. wait for any other packets - wait for a certain time 	- DONE

	********** end after receive *********/

	if ( debug ) {
		// Final adjacency matrix
		printArray(rowCol, adjMat);
	}

	// calculating shortest path & printing forwarding table for router
	djikstra(adjMat, rounterLabel, atoi(totalNumRouters), debug);

	// check if dynamic given in the argument
	if ( dynamic ) {

		/********* start if dynamic *********
			
			To-do:
				0. change a path cost 									- DONE
				1. Make the packet with only one LSP 					- DONE
				2. send the packet to this router's neighbors 			- DONE
				3. other router should wait for any dynamic packet - if no dynamic in the argument - DONE
				4. flooding 											- DONE
				5. make updated adjacency matrix 						- DONE
				6. call djikstra() with the updated adjacency matrix 	- DONE
				7. print the forwarding table 							- DONE

		********** end if dynamic *********/

		// create a dynamic cost between 1-10
		srand(time(NULL));   // should only be called once
		int dynamicCost = ( rand() % 10 ) + 1;

		if ( debug )
			printf("dynamicCost: %d\n", dynamicCost);

		/******** 1. Make the packet with only one LSP ********/

		// check allLSP and make the change to it's first child
		// sending only one LSP - the changed one
		dynamicLSP.numberOfNeighbor = 1;
		// assigning hop count to packet
		dynamicLSP.hopCount = atoi(totalNumRouters) - 1;
		// copy the router source label to the packet
		strcpy(dynamicLSP.source, rounterLabel);

		// copying allLSP's first element to dynamicLSP's first singleLSP - see allLSP struct in node.h
		memcpy(&dynamicLSP.singleLSP[0], &allLSP.singleLSP[0], sizeof(dynamicLSP.singleLSP[0]));

		// assign dynamic cost to dynamicLSP
		// adding the dynamic cost to previous cost
		// current_cost = previous_cost + dynamic_cost
		dynamicLSP.singleLSP[0].cost += dynamicCost;

		// add 1 to present sequence number
		dynamicLSP.singleLSP[0].seqNum++;

		if ( debug ) {
			printf("copied allLSP\n");
			printf("[%d] %s %d %s %s %d %d\n", dynamicLSP.numberOfNeighbor, dynamicLSP.source, dynamicLSP.singleLSP[0].seqNum, dynamicLSP.singleLSP[0].label, dynamicLSP.singleLSP[0].nodeIP, dynamicLSP.singleLSP[0].nodePort, dynamicLSP.singleLSP[0].cost);
		}

		// update own adjacency matrix
		adjMatrixChange( adjMat, dynamicLSP.source, dynamicLSP.singleLSP[0].label, dynamicLSP.singleLSP[0].cost );

		if ( debug ) {
			// adjacency matrix with dynamic cost
			printf("adjacency matrix with dynamic cost\n");
			printArray(rowCol, adjMat);
		}

		/******** 2. send the packet to this router's neighbors  ********/
		for ( j = 0; j < neighborCounter; j++ ) {
			
			// size of sockaddr_in
			addrlen = sizeof(neighbors[j]);

			// send the "dynamicLSP" to other routers
			sendRet = sendto( nodeSd, &dynamicLSP, sizeof(dynamicLSP), 0, (struct sockaddr*)&neighbors[j], addrlen);

			// error checking
			if ( sendRet < 0 ) {
				perror("something went wrong while sending:");
				exit(1);
			}

		}


	}

		/********* start if not dynamic *********

			To-do:
				1. wait for any incoming dynamic packets
		
		********** end if not dynamic *********/

	printf("\nWaiting for any dynamic cost change in router.\n");

	// receive & send packet
	floodReceiveWithSelect( nodeSd, neighbors, rowCol, adjMat, neighborCounter, seqMat, debug );

	// printing final adjacency matrix with dynamic cost

	if ( debug ) {
		printf("Final adjacency matrix with dynamic cost change\n");
		printArray(rowCol, adjMat);

		printf("Sequence matrix\n");
		printArrayArray(rowCol, seqMat);
	}

	// calculating shortest path & printing forwarding table for router
	djikstra(adjMat, rounterLabel, atoi(totalNumRouters), debug);


	
	fclose(fp);
	return 0;
}















