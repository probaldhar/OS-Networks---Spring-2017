/**
 * Implements all operation for bbserver.
 *
 * @author : Probal chandra dhar
 * @Filename : server.c
 * @Date : 03/01/17
 * @course : COP5990
 * @Project # : 3
 * @Usage: ./bbserver <serever's port no> <numberHosts>
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/wait.h>
#include	<fcntl.h>
#include	<signal.h>
#include 	<netdb.h>
#include 	<unistd.h>

#define BUFSIZE 512

int main(int argc,  char *argv[] )

{
	int sd, numbytes,bytessent, ret;
	struct sockaddr_in serveraddress, cliaddr;
	socklen_t length, addressSize;
	char datareceived[BUFSIZE];
	struct hostent *hostptr;
    char hostname[128];

   	struct sockaddr_in allAddress[10];

	if ( argc < 3 ){
		printf("Usage: bbserver <portNum> <numberHosts>\n");
		exit(1);
	}

	int serverPort = atoi(argv[1]);

	int numberHosts = atoi(argv[2]);

	sd = socket( AF_INET, SOCK_DGRAM, 0 );
	if(0 > sd ) 
	{
		perror( "socket" );
		exit( 1 );
	}

	// Get information about destination host:
	if ( gethostname(hostname, sizeof(hostname)) == -1 ){
		printf("Error\n");
	}

	// get network host entry
    hostptr = gethostbyname(hostname);

    if ( hostptr == NULL ){
    	perror("NULL:");
    	return 0;
    }

	memset( (void *) &serveraddress, 0, sizeof(serveraddress) );
	memset( &cliaddr, 0, sizeof(cliaddr) );
	serveraddress.sin_family = (short) AF_INET;
	serveraddress.sin_port = htons((u_short)serverPort);//PORT NO
	serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);//IP ADDRESS

	memcpy((void *)&serveraddress.sin_addr, (void *)hostptr->h_addr, hostptr->h_length);

	printf("name: %s\n", hostptr->h_name);
    printf("addr: [%s]\n", inet_ntoa(serveraddress.sin_addr));
   	printf("port: %d\n",ntohs(serveraddress.sin_port));

	ret = bind(sd, (struct sockaddr *) &serveraddress, sizeof(serveraddress) );
	
	if (0 > ret)
	{
		perror("Bind Fails:");
		exit(1);
	}

	int i;

	i = 0;

	for( i = 0; i < numberHosts; i++ ) {

		printf("UDP Server:  Waiting for client data\n");
		
		length = sizeof(cliaddr);
		
		/*Received a datagram*/
		numbytes = recvfrom (sd, datareceived, BUFSIZE, 0, (struct sockaddr*) &cliaddr, &length);

		if ( numbytes < 0 )
		{
			perror("Error while receiving:");
			exit(1);
		}

		allAddress[i] = cliaddr;
		
		/*Printing client's address*/
		// printf("Data Received from %s\n", inet_ntop (AF_INET, &cliaddr.sin_addr, clientname, sizeof(clientname) ) );
		
		printf("Data Received from %s\n", inet_ntoa(cliaddr.sin_addr) );
		
		// printf("%d\n", ntohs(serveraddress.sin_port));

		/*Sending the Received datagram back*/
		datareceived[numbytes]='\0';
		printf("Server Receives: %s\n", datareceived);

		strcpy(datareceived, "connected");
		
		bytessent = sendto(sd, datareceived, BUFSIZE, 0, (struct sockaddr *) &cliaddr, length);
		
		if ( bytessent < 0 )
		{
			perror("Error while sending:");
			exit(1);
		}
	}

	int addressSend;

	for( i = 0; i < numberHosts; i++ ) {

		// printing the IP addresses & ports
		printf("Data Received from %s\n", inet_ntoa(allAddress[i].sin_addr) );

		printf("%d\n", ntohs(allAddress[i].sin_port));

		// getting the size of the sockaddr_in struct
		addressSize = sizeof(allAddress[i]);

		if ( i != numberHosts - 1 ) {

			// sending the next peer's address to the previous peer
			addressSend = sendto(sd, &allAddress[i+1], sizeof(allAddress[i+1]), 0, (struct sockaddr *) &allAddress[i], addressSize);
		
			if ( addressSend < 0 )
			{
				perror("Error while sending IP address:");
				exit(1);
			}


		}
		else {

			// sending the first address to the last peer
			addressSend = sendto(sd, &allAddress[0], sizeof(allAddress[0]), 0, (struct sockaddr *) &allAddress[i], addressSize);
		
			if ( addressSend < 0 )
			{
				perror("Error while sending IP address:");
				exit(1);
			}
		}

	}



	/*
		no need to send "start" to 1st peer anymore
	*/

	// strcpy(gameStart, "start");

	// addressSize = sizeof(allAddress[0]);

	// // choose the first peer and send "start"
	// addressSend = sendto(sd, gameStart, sizeof(gameStart), 0, (struct sockaddr *) &allAddress[0], addressSize);
		
	// if ( addressSend < 0 )
	// {
	// 	perror("Error while sending IP address:");
	// 	exit(1);
	// }


	// closing the socket
	close(sd);
	return 0;

}

