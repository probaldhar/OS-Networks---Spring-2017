/**
 * functions for node.
 *
 * @author : Probal
 * @Filename : functions.c
 * @Date : 04/20/17
 * @course : COP5990
 * @Project # : 5
 *
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


/**
 * changing the matrix value with respect to routerLabel
 * routerLabel must be one character, starting from A - must be capital
 * Formula used: routerLabel % 65
 *
 * @param   adjMat - adjacency matrix
 * @param   sourceRouter - label of source the router
 * @param   labelRouter - label of dest the router
 * @param   cost - cost of a path
 *
 */
void adjMatrixChange( int **adjMat, char *sourceRouter, char *labelRouter, int cost ) {

	// value of source & destination routerLabel
	int source, dest;
	
	// integer value of source routerLabel
	source = sourceRouter[0] % 65;

	// integer value of dest routerLabel
	dest = labelRouter[0] % 65;
	
	// changing the matrix value
	adjMat[source][dest] = cost;
	adjMat[dest][source] = cost;

}


void seqMatrixChange( int n, int seqMat[][n], char *sourceRouter, char *labelRouter, int seqNum ) {

	// value of source & destination routerLabel
	int source, dest;
	
	// integer value of source routerLabel
	source = sourceRouter[0] % 65;

	// integer value of dest routerLabel
	dest = labelRouter[0] % 65;
	
	// changing the matrix value
	seqMat[source][dest] = seqNum;
	seqMat[dest][source] = seqNum;

}

/**
 * Printing the current adjacency matrix pointer
 *
 * @param   n - number of row & column in the adjacency matrix
 * @param   array - the adjacency matrix
 *
 */
void printArray ( int n, int **array ) {
	int i, j;
	
	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < n; j++ ) {
			if (array[i][j] != inf )
				printf("%5d ", array[i][j]);
			else 
				printf("  inf ");
		}
		printf("\n");		
	}
}

/**
 * Printing the current sequence matrix using array
 *
 * @param   n - number of row & column in the sequence matrix
 * @param   array - the sequence matrix
 *
 */
void printArrayArray ( int n, int array[][n] ) {
	int i, j;
	
	for ( i = 0; i < n; i++ ) {
		for ( j = 0; j < n; j++ ) {
			if (array[i][j] != inf )
				printf("%5d ", array[i][j]);
			else 
				printf("  inf ");
		}
		printf("\n");		
	}
}

/**
 * calculating minimum between two values
 *
 * @param   x - first value
 * @param   y - second value
 *
 * @return	y if x>y and x otherwise
 *
 */
int min ( int x, int y )
{
	return ( x>y ? y:x );
}


/**
 * calculating shortest path for a router to go to others router 
 * and priting the forwaring table using djikstra's algorithm.
 *
 * @param   adjMat 			- adjacency matrix
 * @param   rounterLabel 	- source router label
 * @param	totalNumRouters	- total number of router
 * @param   debug           - printing infomation to the screen
 *
 */
void djikstra ( int **adjMat, char *rounterLabel, int totalNumRouters, int debug ) {

	// cost matrix
	struct node costMat[totalNumRouters+1][totalNumRouters];
	// visited nodes
	int visited[totalNumRouters];
	// Router rootNode
	int vIndex; 
	vIndex = rounterLabel[0] % 65;
	int minIndex,minVal;
	
	int i,j;
	int markedVal;

	// initializing visited matrix to 0
	for ( i = 0; i < totalNumRouters; i++ )
		visited[i] = 0;
	
	
	// fill the first row of the cost matrix as infinity
	for ( i = 0; i < totalNumRouters; i++ )
		if ( i == vIndex )
			costMat[0][i].cost = 0; 
		else 
			costMat[0][i].cost = inf;
			
	markedVal = 0;

	for ( i = 1; i < totalNumRouters + 1; i++ )
	{
		visited[vIndex] = 1; // mark the first node as visited
		minVal = inf;
		/**scan through the matrix for neighbouring nodes and update the cost and the previous visited node 
         If the node is already visited just copy the previous row values
         else
         calculate the minimum cost for each iteration
       */
       
		for( j = 0; j < totalNumRouters; j++ )
		{
			if ( visited[j] == 1 )
			{	
				costMat[i][j].cost = costMat[i-1][j].cost;
				costMat[i][j].prev = costMat[i-1][j].prev;
				
				continue;
			// if the node is already visited, just move on to the next node
			} else {
				if ( min( markedVal+adjMat[vIndex][j],costMat[i-1][j].cost) == costMat[i-1][j].cost)
				{
					costMat[i][j].cost = costMat[i-1][j].cost;
					costMat[i][j].prev = costMat[i-1][j].prev;
				} else {
						costMat[i][j].prev = vIndex;
						costMat[i][j].cost = min(markedVal+adjMat[vIndex][j],costMat[i-1][j].cost);
				}

            /**This is used to select the next node to be visited*/
				if ( costMat[i][j].cost < minVal )
				{
					minVal = costMat[i][j].cost;
					minIndex = j;
				}
			}
			
			
			
		}

		// the next node to be visited is updated
      	vIndex = minIndex;
		markedVal = minVal;

	}
	
	// there's no previous so that making that -1
	costMat[totalNumRouters][rounterLabel[0] % 65].prev = -1;
	
	if ( debug ) {
	   	// printing the cost Matrix
		for ( i = 1; i < totalNumRouters+1; i++ )
		{
			for(j=0;j<totalNumRouters;j++)
			{
				if ( costMat[i][j].cost != 5000 && j != vIndex ) 
					printf("%d %c  ",costMat[i][j].cost,costMat[i][j].prev+65);
				else 
					printf("inf");
			}
			
			printf("\n");
		}
	}
		
		
	/** the path to be searched for a specific router */
	
	printf("\nForwarding table for: %s\n\n", rounterLabel);

	// loop varialbe
	int search;

	for ( search = 0; search < totalNumRouters; search ++ ) {

		i = totalNumRouters;
		j = search;

		/** the loop is used to backtrack from the destination node to the source node using the previous node information */
      	while ( costMat[i][j].prev != -1 && costMat[i][costMat[i][j].prev].cost!=0) {
			j = costMat[i][j].prev;
		}

		// not printing present router's next hop
		if ( (rounterLabel[0] % 65) != j ) 
			printf("The router hop to %c is %c\n", 65+search, j+65);

	}
		
	
}




/**
 * flooding packets, receiving with system call select() to wait for a certain time
 *
 * @param   nodeSd          - socket descriptor
 * @param   neighbors[]     - array of nrighbor router connection
 * @param   rowCol          - number of router
 * @param   adjMat          - adjacency matrix
 * @param   neighborCounter - number of neighbor of a router
 * @param   debug           - printing infomation to the screen
 *
 */
void floodReceiveWithSelect( int nodeSd, struct sockaddr_in neighbors[NUM_NEIGHBOR], int rowCol, int **adjMat, int neighborCounter, int seqMat[][rowCol], int debug ) {

	// select params
	fd_set readfds;
	struct timeval tv;

	int i, j, numfd, retval, recvlen, sendRet, receivedNeighborCounter, source, dest;
	// length of sockaddr_in
	socklen_t addrlen;
	// Link state packets for each record in the file
	allLSP recvLSP;
	// sockaddr_in to send the message
	struct sockaddr_in recvAddress;

	/***** LOOP START *****/ // for receive & send

	while (1) {

		// select system call
		// add our descriptors to the set
		FD_ZERO(&readfds);
	    FD_SET(nodeSd, &readfds);

	    // the highest-numbered file descriptor
	    numfd = nodeSd + 1;

	    /* Wait up to five seconds. */
	    tv.tv_sec = TIMEOUT;
	    tv.tv_usec = 0;

	    // checking return value of select
	    retval = select(numfd, &readfds, NULL, NULL, &tv);
	    /* Don't rely on the value of tv now! */

	    if (retval == -1)
	    	perror("select()");
	    else if (retval) {

	    	if ( debug ) 
	    		printf("Data is available now.\n");

	        /*******  receive & send  ******/

	        // size of sockaddr_in
			addrlen = sizeof(recvAddress);

			// should wait for any receive message and send if hop > 0

			// receive "recvLSP" from other nodes
			recvlen = recvfrom( nodeSd, &recvLSP, sizeof(recvLSP), 0, (struct sockaddr*)&recvAddress, &addrlen) ;
			
			// error check
			if ( recvlen < 0 ) {
				perror("something went wrong while receiving:");
				exit(1);
			}

			// LSP counter in the packet received
			receivedNeighborCounter = recvLSP.numberOfNeighbor;

			// printing what received
			for ( j = 0; j < receivedNeighborCounter; j++ ) {

				if ( debug )
					printf("[%d] %s %d %s %s %d %d\n", recvLSP.numberOfNeighbor, recvLSP.source, recvLSP.singleLSP[j].seqNum, recvLSP.singleLSP[j].label, recvLSP.singleLSP[j].nodeIP, recvLSP.singleLSP[j].nodePort, recvLSP.singleLSP[j].cost);

				// calculating matrix position
				source = recvLSP.source[0] % 65;
				dest = recvLSP.singleLSP[j].label[0] % 65;
				
				if ( recvLSP.singleLSP[j].seqNum != seqMat[source][dest] ) { 
					
					// reform matrix
					adjMatrixChange( adjMat, recvLSP.source, recvLSP.singleLSP[j].label, recvLSP.singleLSP[j].cost );

					seqMatrixChange( rowCol, seqMat, recvLSP.source, recvLSP.singleLSP[j].label, recvLSP.singleLSP[j].seqNum );
				
				}

			}

			if ( debug ) {
				// printing adjacency matrix so far
				printArray(rowCol, adjMat);
			}

			// check the hop count & send

			// check the hop count & change the hop count(s) if necessary
			if ( recvLSP.hopCount > 0) { 

				if ( debug ) 
					printf("Hop Count: %d\n", recvLSP.hopCount);

				// reducing the hop count for a packet
				recvLSP.hopCount--;

				// send the packet to other neighbors
				for ( i = 0; i < neighborCounter; i++ ) {

					// check if the same source from where I got the packet
					if ( !strcmp (inet_ntoa(recvAddress.sin_addr), inet_ntoa(neighbors[i].sin_addr) ) && (ntohs(neighbors[i].sin_port) == ntohs(recvAddress.sin_port)) ) {
						// dont't send - because that's where you got the packet
					} else {
						// send the packet to other neighbors

						// size of sockaddr_in
						addrlen = sizeof(recvAddress);

						// send tha packet
						sendRet = sendto( nodeSd, &recvLSP, sizeof(recvLSP), 0, (struct sockaddr*)&neighbors[i],addrlen);

						// error checking
						if ( sendRet < 0 ) {
							perror("something went wrong while sending:");
							exit(1);
						}
					}
				}

			} // end if

	    } else {
	    	printf("No more packet received within %d seconds.\n", TIMEOUT);
	    	// break the while loop
	    	break;
	    }

	} // end of while(1)
	/***** LOOP END *****/ // for receive & send

}




