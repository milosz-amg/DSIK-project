#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define PORT 13373
#define BUFSIZE 256
#define NUM_CL 2

void setLocalSocAddr(struct sockaddr_in *serverAddr,
int port) {
	(*serverAddr).sin_family = AF_INET;
	(*serverAddr).sin_port = htons(port);
	(*serverAddr).sin_addr.s_addr = 0;
	memset((*serverAddr).sin_zero, 0, strlen((*serverAddr).sin_zero));
}
// allows program to die verbose + safely
void error (char * msg) {
	perror(msg);
	exit (1);
}

// sends a message to a client based on sock no. and message string
void sendMessage(int client, char *msg) {
	int msgSize = strlen(msg);
	if (send(client, msg, msgSize, 0) != msgSize)
		error( "ERROR on send");
}

void main (int argc, char *argv[]) {
	int sockid, clientLen, msgSize;
	struct sockaddr_in serverAddr;

	int clientSock[NUM_CL] ;
	int currClients = 0;
	struct sockaddr_in clientAddr[NUM_CL];
	char echoBuf[BUFSIZE] ;
	
	// initial setup
	// create socket
	sockid = socket(PF_INET, SOCK_STREAM, 0);

	if (sockid < 0) {
		error( " ERROR opening socket");
	}
	
	// setup server address/ format
	setLocalSocAddr(&serverAddr, PORT);

	// bind to network addr and listen on port
	if (bind(sockid, (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
		error( "ERROR on bind");
	}
	if (listen(sockid, 10) < 0) {
		error( "ERROR on listen");
	}

	// complete!
	printf ("Server has been successfully initialized (socket, bind, listen)\n");

// start a loop of accepting client connections and then receiving messages
//while (1) {

	// waiting for all of the clients to join
	while (currClients < NUM_CL) {
		clientLen = sizeof (clientAddr[currClients] ) ;
		// accept clients (creates a new socket)
		if ( (clientSock[currClients]=accept(sockid,(struct sockaddr *)&clientAddr[currClients], &clientLen)) < 0) {
			error("ERROR on accept");
		}
		currClients ++;
		printf( "We have accepted client number %d\n" , currClients) ;
	}
	
	// sends a message to all users
	int i;
	for (i = 0; i < NUM_CL; ++i) {
		char *a = "\nThe game has begun. Please wait.\nWhen your turn comes, you can play: 1 [rock], 2 [paper], or 3 [scissors].\n\n";
		sendMessage(clientSock[i], a);
	}
	
	// goes user by user
	int moves[NUM_CL];
	for (i = 0; i < NUM_CL; ++i) {
		// user's turn
		sendMessage(clientSock[i], "\nIt is your turn!\n> ");
		if ((msgSize = recv(clientSock[i], echoBuf, BUFSIZE, 0)) < 0) {
			error("ERROR on recv 2");
		}
		// user completed move
		sendMessage(clientSock[i], "Thank you for your play\n");
		// grab # (1-3 mapped to 0,1,2)
		moves[i] = echoBuf[0] - '1';
		moves[i] %= 3;
		
		// report to other players that a move has been done
		int j;
		for (j = 0; j < NUM_CL; ++j) {
			if (i != j) sendMessage(clientSock[j], "A player has played.\n");
		}
	}
	
	// store states, does some cool math to figure out who lost and who won
	// if end result is 0, same move = draw
	// if end result is 1, they lost
	// if end result is 2, they won
	int numStates[2];
	numStates[0] = moves[1] - moves[0];
	numStates[1] = -numStates[0];
	if (numStates[0] < 0) {
		numStates[0] += 3;
	} else {
		numStates[1] += 3;
	}
	char *textStates[3] = {"drew", "lost", "won" } ;

	// end game, send out messages
	printf ("Closing connection with clients\n");
	for (i = 0; i < NUM_CL; ++i) {
		char endGame[36] ;
		char *end = textStates[numStates[i]];
		// format statement into string to send out
		// stores data in endGame var
		snprintf(endGame, 36, "You %s\n\nGoodbye!\n" ,end);
		sendMessage(clientSock[i], endGame) ;
		close(clientSock[i]);
	}
}