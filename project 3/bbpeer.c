/**
 * Implements all operation for bbpeer.
 *
 * @author : Probal chandra dhar
 * @Filename : peer.c
 * @Date : 03/01/17
 * @course : COP5990
 * @Project # : 3
 * @Usage: ./bbpeer [-new] <portNum> <hostIP> <hostPort> <filenameBulletinBoard>
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
#include	"peer.h"

// global variables
/*
	doWeNeedToken - 0 if token not needed, 1 if file needs to be written, 2 if read a message, 3 if list the sequence number
*/
int doWeNeedToken = 0, mainLoop = 0, newPosition, currentStream, readMsgNumber;

char globalSharedBuffer[BUFSIZE], *printmessage, *readMessage;

// need to decleare the dest and prev as global
// so that anyone can join in the middle of the ring and exit from the ring
struct sockaddr_in *dest;
struct sockaddr_in prev, ownAddress;


int main( int argc, char *argv[] )
{
	/*
		sd				- socket descriptor
		ret				- sendto/recvfrom return value
		recvIPaddress	- sendto/recvfrom return value
		sendMessage		- sendto/recvfrom return value
		threadErr		- pthread_create return value
		actionNumber	- get user menu number from user
	*/
	int	sd, ret, recvIPaddress, sendMessage, threadErr, actionNumber;
	// int sendNumber = 9999;

	/*
		serveraddress	- server's descriptor (IP, PORT etc)
		ownAddress		- own descriptor (IP, PORT etc)
	*/
	struct sockaddr_in serveraddress;
	// host information
	struct hostent *hostptr;
	// length of dest & prev descriptor
	socklen_t IPaddrLength, IPPrevLength;
	// used for storing the host information, token and IPchange message
	char hostname[128], recvNumberOrStart[128], token[10], IPchange[BUFSIZE];
	// char sendNumberString[10];
	// storing the arguments
	char *portNum, *hostIP, *hostPort, *filenameBulletinBoard;

	// setting memory for the argument strings separately
	portNum = (char * )malloc(sizeof(char)*MAXSERVERNAME);
	hostIP = (char * )malloc(sizeof(char)*MAXSERVERNAME);
	hostPort = (char * )malloc(sizeof(char)*MAXSERVERNAME);
	filenameBulletinBoard = (char * )malloc(sizeof(char)*MAXSERVERNAME);

	// setting the memory for the shared global buffer to store the message
	// globalSharedBuffer = (char *)malloc(sizeof(char)*BUFSIZE);

	// thread ID
	pthread_t tid;

	// initialize semaphore
    sem_init(&semaphore, 0, 0);

	// USAGE
    if ( argc < 5 ) {
    	printf("Usage: bbpeer [-new] <portNum> <hostIP> <hostPort> <filenameBulletinBoard>\n");
    	exit(1);
    }

	// setting the arguments as variables
    if ( !strcmp(argv[1], "-new") ) {

    	portNum = argv[2];
    	hostIP = argv[3];
    	hostPort = argv[4];
    	filenameBulletinBoard = argv[5];

    } else {
    	portNum = argv[1];
    	hostIP = argv[2];
    	hostPort = argv[3];
    	filenameBulletinBoard = argv[4];
    	// printf("%s %s %s\n", portNum, hostIP, hostPort);
    }

	// checking the file pointer 
	FILE * fp;

	// opening the file
	fp = fopen(filenameBulletinBoard, "r+" );

	// checking if the file is open or not
	if ( fp == NULL ){
		printf("FILE not open. Please create a file first.\n");
		exit(1);
	}
	// end checking the file pointer 
	
	// crating the socket
	sd = socket( AF_INET, SOCK_DGRAM, 0 );
	// error checking
	if ( sd < 0 ) 
	{
		perror( "socket: " );
		exit (1);
	}

	// Get information about client:
	if ( gethostname(hostname, sizeof(hostname) ) ){
		printf("Error\n");
	}

	// get network host entry
    hostptr = gethostbyname(hostname);
	
	// server's address
	memset( &serveraddress, 0, sizeof(serveraddress) );
	serveraddress.sin_family = AF_INET;
	serveraddress.sin_port = htons(atoi(hostPort));//PORT NO
	serveraddress.sin_addr.s_addr = inet_addr(hostIP);//ADDRESS

	// setting memory to prev 
	memset( &prev, 0, sizeof(prev) );


	// peer's address
	memset( &ownAddress, 0, sizeof(ownAddress) );
	ownAddress.sin_family = AF_INET;
	ownAddress.sin_port = htons(atoi(portNum));//PORT NO
	ownAddress.sin_addr.s_addr = htonl(INADDR_ANY);//ADDRESS

	memcpy((void *)&ownAddress.sin_addr, (void *)hostptr->h_addr, hostptr->h_length); 

	// printing the hostname, IP & port
	printf("name: %s\n", hostptr->h_name);
    printf("addr: [%s]\n", inet_ntoa(ownAddress.sin_addr));
    printf("port: %d\n",ntohs(ownAddress.sin_port));
	
	printf("Peer Starting service\n");

	// binding to a specific port
	if ( bind(sd, (struct sockaddr *) &ownAddress, sizeof(ownAddress) ) < 0 ){
		perror("Bind Fails:");
		exit(1);
	}

	// calling function to send and recv the first batch of datagram
	ret = main_functionlity(stdin, sd, (struct sockaddr *) &serveraddress, sizeof(serveraddress));
	
	if ( ret < 0 )
	{
		printf("Client Exiting - Some error occured\n");
		close(sd);
		exit(1);
	}


	// IP address length
	IPaddrLength = sizeof(serveraddress);

	dest = (struct sockaddr_in *)malloc(sizeof(dest));

	// getting the IP address
	recvIPaddress = recvfrom (sd, dest, sizeof(dest), 0, (struct sockaddr*) &serveraddress, &IPaddrLength);

	if ( recvIPaddress < 0 )
	{
		perror("Error while receiving:");
		exit(1);
	}

	// printing received IP & port
	printf("Received next peer's address & port.\n");

	printf("IP: %s\n", inet_ntoa(dest->sin_addr) );

	printf("PORT: %d\n", ntohs(dest->sin_port));


	/*
		============ Game start ===================
	*/

	// port number % 60000
	int portForGame = ntohs(ownAddress.sin_port) % 60000;

	int initialGameNumber = 0, recvGameNumber = 0; 

	// exit(1);

	IPaddrLength = sizeof(*dest);
	IPPrevLength = sizeof(prev);

	// check for second argument if that's "-new"
	if ( !strcmp(argv[1], "-new") ) {
		// joining peer

		// receive "join"
		recvIPaddress = recvfrom (sd, recvNumberOrStart, sizeof(recvNumberOrStart), 0, (struct sockaddr*) &serveraddress, &IPaddrLength);

		if ( recvIPaddress < 0 )
		{
			perror("Error while receiving:");
			exit(1);
		}

		// checking the message if it's "join"
		if ( !strcmp(recvNumberOrStart, "join") ) {

			// previous peer's address, so store it in prev
			// prev = serveraddress;

			// assigning sockaddr_in will not work, used memcpy
			memcpy(&prev, &serveraddress, sizeof(prev));

			printf("IP: %s\n", inet_ntoa(prev.sin_addr) );

			printf("PORT: %d\n", ntohs(prev.sin_port));

			/* 	Anything else needed for joined peer? 
				Things done: 
					1. received dest adddress & port
					2. set prev address & port - for exiting

				Anything else - NO!
			*/

		} else {
			// checking for error
			printf("received while joining new peer: %s", recvNumberOrStart);
		}


	} else { 
		// start game when creating ring from server
		while (1) {

			// send initialGameNumber to the destination (next peer)
			sendMessage = sendto(sd, &initialGameNumber, sizeof(int), 0, (struct sockaddr *) dest, IPaddrLength);
			// error checking
			if ( sendMessage < 0 )
			{
				perror("Error while sending IP address:");
				exit(1);
			}

			// getting game's number or "start"
			recvIPaddress = recvfrom (sd, &recvGameNumber, sizeof(int), 0, (struct sockaddr*) &prev, &IPPrevLength);
			// error checking
			if ( recvIPaddress < 0 )
			{
				perror("Error while receiving:");
				exit(1);
			}

			// compare the number with the portForGame number
			if ( recvGameNumber == portForGame ) {
				printf("I win, passing token start\n");

				// if win then send -1 to the next peer, so the loop can stop
				initialGameNumber = -1;

				// sending -1
				sendMessage = sendto(sd, &initialGameNumber, sizeof(int), 0, (struct sockaddr *) dest, IPaddrLength);
				
				if ( sendMessage < 0 )
				{
					perror("Error while sending IP address:");
					exit(1);
				}

				// sendto next peer a token

				// T - is my TOKEN
				strcpy(token, "T");

				IPaddrLength = sizeof(*dest);

				// send token to the next peer
				sendMessage = sendto(sd, token, sizeof(token), 0, (struct sockaddr *) dest, IPaddrLength);
				// error checking
				if ( sendMessage < 0 )
				{
					perror("Error while sending TOKEN:");
					exit(1);
				}

				// printf("token sent");

				break;
				
			} else if ( recvGameNumber == -1 ) {
				// if -1 received, that mean winner already selected, so need to break the loop
				// printf("break the loop\n");
				initialGameNumber = -1;
				sendMessage = sendto(sd, &initialGameNumber, sizeof(int), 0, (struct sockaddr *) dest, IPaddrLength);
				// error checking
				if ( sendMessage < 0 )
				{
					perror("Error while sending IP address:");
					exit(1);
				}
				
				break;

			} else if ( recvGameNumber > 9999 ){
				// if some garbage value received
				recvGameNumber = 0;
				initialGameNumber = 0;
			} else {
				// increase the number
				initialGameNumber++;
			}

		}

	} // end else

	/*
		============ Game end ===================
	*/


	/*
		============ thread create start ===================
	*/

	// thread argument struct
	threadArgs args;
	args.dest = dest;
	args.socketID = sd;
	args.filefp = fp;

	// thread create for token passing
	threadErr = pthread_create(&tid, NULL, threadFunc, (void *) &args ); // dest already a pointer
        	
	if (threadErr != 0) {
		printf("\ncan't create thread :|%s|", strerror(threadErr));
	}

	/*
		============ thread create end ===================
	*/



	/*
		============ User manu start ===================
	*/

	// loop variables
	int insideLoopVar = 1, i;

	// allocating memory
	readMessage = (char *)malloc(sizeof(char)*MESSAGE);

	// loop to get input from user
	while( insideLoopVar ) { 

		// char pointer for the main message
		printmessage = (char *)malloc(sizeof(char) * 100);

		// so that token can pass through the ring
		// doWeNeedToken = 0;

		// USER input menu
		printf("\n------------------------------------\n");
		printf("Choose from menu:\n");
		printf("1. write\n");
		printf("2. read #\n");
		printf("3. list\n");
		printf("4. Exit\n");
		printf("5. print dest information\n");
		printf("6. print prev information\n");
		printf("------------------------------------\n");
		printf("Enter your choice (number): ");

		// getting number from user from the menu
		scanf("%d", &actionNumber);
		printf("You choose: %d\n", actionNumber);
		getchar(); // eat the newline character

		/*
			case 1: write a message to the file
			case 2: read a message from file
			case 3: print all sequence number of the messages
			case 4: EXIT
			case 5: print next peer's information
			case 6: print previous peer's information
		*/
		switch(actionNumber) {

			case 1: 

				printf("Enter your message\n");

				// getting the message
				if ( fgets(printmessage, MESSAGE, stdin) == NULL )
					printf("NULL\n");

				// fgets adds a "\n" in the string, removing it using strtok
				strtok(printmessage, "\n");

				printf("Your message: %s", printmessage);

				// assigned to write the file
				doWeNeedToken = 1;

				// block the user
				sem_wait(&semaphore);

				break;
			case 2: 

				printf("Enter the number of the message you want to read from bulletin board:\n");
				scanf("%d", &readMsgNumber);

				printf("You entered: %d\n", readMsgNumber);

				// assigned to read the file
				doWeNeedToken = 2;

				// block the user
				sem_wait(&semaphore);

				break;
			case 3:

				printf("Sequence numbers of the messages are shown below:\n");

				// assigned to read the file
				doWeNeedToken = 3;

				// block the user
				sem_wait(&semaphore);

				// printing the valid sequence numbers of messages posted on the board
				if ( newPosition > 0 ) {
					for ( i = 0; i < newPosition; i++ )
						printf("%d ", i+1);
					printf("\n");
				} else {
					// in case there's no message in the file
					printf("There is no message stored in the file.\n");
				}

				break;
			case 4: 
				printf("Exiting program\n");

				// check if only peer only in the ring
				if ( !strcmp (inet_ntoa(prev.sin_addr), inet_ntoa(dest->sin_addr) ) && (ntohs(dest->sin_port) == ntohs(prev.sin_port)) && !strcmp (inet_ntoa(prev.sin_addr), inet_ntoa(ownAddress.sin_addr) ) && (ntohs(prev.sin_port) == ntohs(ownAddress.sin_port)) ) {

					printf("Peer Exiting ring.\n");

					// setting the thread loop global variable to 1 to stop
					mainLoop = 1;
					
					// breaking the loop
					insideLoopVar = 0;
					
					break;

				}

				// sending IP & port to the prev peer
				strcpy(IPchange, "IPchange");

				IPaddrLength = sizeof(prev);

				// send "IPchange" to the prev (previous peer)
				sendMessage = sendto(sd, IPchange, sizeof(IPchange), 0, (struct sockaddr *) &prev, IPaddrLength);
					
				// error checking
				if ( sendMessage < 0 )
				{
					perror("Error while sending IPchange to the prev peer:");
					exit(1);
				}

				// send dest's IP & port to the prev peer (previous peer) 
				sendMessage = sendto(sd, dest, sizeof(dest), 0, (struct sockaddr *) &prev, IPaddrLength);
					
				// error checking
				if ( sendMessage < 0 )
				{
					perror("Error while sending \"dest\" to the prev peer:");
					exit(1);
				}

		// printf("addr: [%s]\n", inet_ntoa(dest->sin_addr));
		// printf("port: %d\n",ntohs(dest->sin_port));

				printf("Peer Exiting ring.\n");

				// setting the thread loop global variable to 1 to stop
				mainLoop = 1;
				
				// breaking the loop
				insideLoopVar = 0;
				
				break;
			case 5:
				printf("Next peer's IP: %s\n", inet_ntoa(dest->sin_addr) );

				printf("Next peer's PORT: %d\n", ntohs(dest->sin_port));
				break;
			case 6:
				printf("previous peer's IP: %s\n", inet_ntoa(prev.sin_addr) );

				printf("previous peer's PORT: %d\n", ntohs(prev.sin_port));

				break;
			default:
				printf("wrong number\n");
				break;

		}	

	}

	/*
		============ User manu end ===================
	*/

	// exiting the program
	printf("EXIT\n");

	// wait for complition of the thread
	pthread_join(tid, NULL);

	// closing the socket
	close(sd);

	// closing the file
	if (fp)
		fclose(fp);

	return 1;
}


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
int main_functionlity(FILE *fp,	int sockfd, struct sockaddr *to ,socklen_t length)
{

	char sendbuf[BUFSIZE], recvbuf[BUFSIZE],servername[MAXSERVERNAME];
	int ret, numbytes, slen;
	socklen_t structlen;
	struct sockaddr_in serveraddr;
	
	// copyting "new" to sendbuf string
	strcpy(sendbuf, "new");

	// finding the string length
	slen = strlen (sendbuf);
	
	// Sending the read data over socket
	ret = sendto(sockfd, sendbuf, slen, 0, to, length);
	// error checking
	if ( ret < 0 )
	{
		perror("Error in sending data:\n");
		return -1;
	}
	
	printf("Data Sent To Server\n");

	// finding the length of the sockaddr_in(serveraddr)
	structlen = sizeof(serveraddr);

	// receiving message from peer
	numbytes = recvfrom (sockfd, recvbuf, BUFSIZE, 0, (struct sockaddr * ) &serveraddr, &structlen);
	// error checking
	if (0 > numbytes)
	{
		perror("Error in receiving data:\n");
		return -1;
	}

	// printing the data what received from server
	printf("Data Received from server %s: ", inet_ntop(AF_INET, &serveraddr.sin_addr, servername, sizeof(servername)));
	
	// writing to stdout
	printf("%s\n", recvbuf);

	return 0;

}





/**
 * Thread function for token passing, receiving join request etc.
 *
 * @param 	param - parameters for the thread, (socket id).
 *
 */
void* threadFunc ( void* param ) {

	// integer to check for errors
	int recvToken, sendToken, recvIPWhenExit, IPnew;
	// character array to send the token as string
	char recvTokenChar[BUFSIZE];
	// stroing the prev and dest IP & port
	struct sockaddr_in prevStore, exitingPeer;
	// setting 0 to those sockaddr_in
	memset( &prevStore, 0, sizeof(prevStore) );
	memset( &exitingPeer, 0, sizeof(exitingPeer) );
	socklen_t IPTokenLength = sizeof(prev), IPTokenSendLength, IPExitingPeer = sizeof(exitingPeer), joiningIPLength = sizeof(*dest);

	// storing prev so that we can check the IP & port when exit a peer and if a new peer arrived
	prevStore = prev;

	// getting socket descriptor
	threadArgs args = *(threadArgs *) param;

	// getting the size of sockaddr_in ( dest )
	IPTokenSendLength = sizeof(*args.dest);

	// main loop for receiving message
	while( mainLoop == 0 ) {

		// receive token/join/exit from previous/next peer
		// receive message from prev peer
		recvToken = recvfrom (args.socketID, recvTokenChar, BUFSIZE, 0, (struct sockaddr*) &prev, &IPTokenLength);

		// error checking
		if ( recvToken < 0 )
		{
			perror("Error while receiving TOKEN:");
			exit(1);
		}

		// printf("TOKEN or what: %s\n", recvTokenChar);
		// if it's token then send
		if ( !strcmp(recvTokenChar, "T") ) {

			// do I need the token to read/write? If yes then hold the Token by semaphore or mutex
			// otherwise release the token immediately

			// printf("doWeNeedToken: %d ", doWeNeedToken);

			// TOKEN not needed right now
			if ( doWeNeedToken == 0 ) {
				// release the token immediately
				
			} else if ( doWeNeedToken == 1 ) {
				// need to write the file

					// writing the file
					if ( doFileWrite(args.filefp) < 0 )
						printf("something went wrong in doFileWrite()");

				// semaphore signal
				sem_post(&semaphore); 

				// again making doWeNeedToken to 0 for token passing
				doWeNeedToken = 0;

			} else if ( doWeNeedToken == 2 ) {
				// need to read the file

					// writing the file
					if ( doFileRead(args.filefp) < 0 )
						printf("something went wrong in doFileRead()");

				// semaphore signal
				sem_post(&semaphore); 

				// again making doWeNeedToken to 0 for token passing
				doWeNeedToken = 0;

			} else if ( doWeNeedToken == 3 ) {
				// need to display the sequence numbers

					// calculating the file size
					if ( calcFileSize( args.filefp ) < 0 )
						printf("something went wrong in calcFileSize()");
				
				// semaphore signal
				sem_post(&semaphore);

				// again making doWeNeedToken to 0 for token passing
				doWeNeedToken = 0;

			}

			// send token to next peer
			sendToken = sendto(args.socketID, recvTokenChar, sizeof(recvTokenChar), 0, (struct sockaddr *) args.dest, IPTokenSendLength);
				
			// error checking
			if ( sendToken < 0 )
			{
				perror("Error while sending TOKEN:");
				exit(1);
			}

		} else if ( !strncmp(recvTokenChar, "new", 3) ) {
			// new peer arrived, make it next peer
			// change the dest IP & number

			printf("joining peer: \n");
			printf("received: %s\n", recvTokenChar);

			strcpy(recvTokenChar, "connected");
			
			// send "connected"
			IPnew = sendto(args.socketID, recvTokenChar, sizeof(recvTokenChar), 0, (struct sockaddr *) &prev, IPTokenLength);
			
			// error checking
			if ( IPnew < 0 )
			{
				perror("Error while sending \"connected\":");
				exit(1);
			}

			// send dest to new peer so that new peer's dest is not dest
			IPnew = sendto(args.socketID, dest, sizeof(dest), 0, (struct sockaddr *) &prev, IPTokenLength);
		
			// error checking
			if ( IPnew < 0 )
			{
				perror("Error while sending dest IP address:");
				exit(1);
			}

			// now send join to prev: prev is the IP of the newly joined peer, dest & prev is now set to the new peer
			strcpy(recvTokenChar, "join");
			
			// send "join"
			IPnew = sendto(args.socketID, recvTokenChar, sizeof(recvTokenChar), 0, (struct sockaddr *) &prev, IPTokenLength);
			
			// error checking
			if ( IPnew < 0 )
			{
				perror("Error while sending \"join\":");
				exit(1);
			}

			// set the dest, right now dest is the dest of newly added peer's IP
			// dest = &prev;

			// assigning sockaddr_in will not work, used memcpy
			memcpy(dest, &prev, joiningIPLength);

			printf("IP: %s\n", inet_ntoa(dest->sin_addr) );

			printf("PORT: %d\n", ntohs(dest->sin_port));

			// set prev to the current peer, right now prev is the newly added peer's IP
			// prev = prevStore;
			memcpy(&prev, &prevStore, sizeof(prev));

			// should be that's it for the "join"

		} else if ( !strcmp(recvTokenChar, "IPchange") ) {

			printf("IPchange\n");
			
			// received "IPchange" from next peer, so the address we get should store in the "dest" struct
			// getting the IP address
			recvIPWhenExit = recvfrom (args.socketID, dest, sizeof(dest), 0, (struct sockaddr*) &exitingPeer, &IPExitingPeer);

			// error checking
			if ( recvIPWhenExit < 0 )
			{
				perror("Error while receiving dest IP when dest IP is exiting:");
				exit(1);
			}

			printf("IP: %s\n", inet_ntoa(dest->sin_addr) );

			printf("PORT: %d\n", ntohs(dest->sin_port));

			// getting the previous IP & peer to "dest", so no need to change that anymore, it's already changed


			strcpy(recvTokenChar, "T");

			/* checked & working */
			// send token to next peer
			sendToken = sendto(args.socketID, recvTokenChar, sizeof(recvTokenChar), 0, (struct sockaddr *) args.dest, IPTokenSendLength);
				
			// error checking
			if ( sendToken < 0 )
			{
				perror("Error while sending TOKEN:");
				exit(1);
			}

		} 

		// printf("%d\n", mainLoop);
		// just checking
		// sleep(2);

	} // end main loop

	printf("thread END\n");

	// exiting the main thread
	pthread_exit(0);

}




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
int writeFile (char *message, unsigned int index, FILE* fp){

	// seeking file for the message
	fseek(fp, index*(MESSAGE), SEEK_SET);

	// writing the message in the file
	if( fwrite(message, (MESSAGE), 1, fp) ){

		return 0;

	}
	
	return -1;
}


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
int readFile (char *message, unsigned int index, FILE* fp){

	// seeking file for the message
	fseek(fp, index*(MESSAGE), SEEK_SET);
	
	// reading the message from the file
	if( fread(message, (MESSAGE), 1, fp) ){

		return 0;

	}

	return -1;

}

/**
 * function to write the file & display confirmation message
 *
 * @param 	fp - File pointer
 *
 * @return 	0 if successful, -1 otherwise
 *
 */
int doFileWrite(FILE *fp){

	// calculating the file size right now
	if ( calcFileSize(fp) < 0 )
		return -1;

	// message header & footer
	char *str1 = "<message n=", *str3 = ">", *str5 = "</message>";

	// message n number
	char *str2 = malloc(sizeof(char) * 100);

	// converting message n number to string
	if ( sprintf(str2, "%d", (newPosition+1) ) < 0 )
		return -1;

	// adding header & footer to string
	if ( snprintf(globalSharedBuffer, sizeof(globalSharedBuffer), "%s%s%s%s%s", str1, str2, str3, printmessage, str5) < 0 )
		return -1;

	// printing the message to screen
	printf("%s\n", globalSharedBuffer);

	// free the string variable
	// free(printmessage); // no need now because already copied buffer to globalSharedBuffer

	// writting File
	if ( !writeFile (globalSharedBuffer, newPosition, fp) ) {
		printf("File written.\n" );
		return 0;
	} else {
		printf("File write failed.\n");
		return -1;
	}

	// make compiler happy :D
	return -1;

}

/**
 * function to read the file & display the message
 *
 * @param 	fp - File pointer
 *
 * @return 	0 if successful, -1 otherwise
 *
 */
int doFileRead( FILE *fp ) {

	if( calcFileSize( fp ) < 0 )
		return -1;
	
	// checking if the desired indexed message is in the file or not
	if ( readMsgNumber > newPosition ) {
		printf("Your desired numbered message is not in the file.");
		return 0;
	}

	// reading the file for the message
	if ( !readFile (readMessage, readMsgNumber-1, fp) ) {
		printf("Your desired message: %s\n", readMessage);
		return 0;
	} else {
		printf("Message can't be read.\n");
		return -1;
	}

	// make compiler happy :D
	return -1;

}

/**
 * function to calculate the file size
 *
 * @param 	fp - File pointer
 *
 * @return 	index if successful, -1 otherwise
 *
 */
int calcFileSize( FILE *fp ) {

	// setting file pointer to the end of the file
	fseek(fp, 0, SEEK_END);

	// right now stream is in the end of the file
	currentStream = ftell(fp);

	if ( currentStream < 0 )
		return -1;

	// finding the number of messages is in the file
	newPosition = currentStream/(MESSAGE);

	// set the stream to the beginning of the file
	fseek(fp, 0, SEEK_SET);

	// returning the index
	return newPosition;

}