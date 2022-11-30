#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>

#define PORT "9032"
#define BUFSIZE 512

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    //checking if it is IPv4
	if(sa->sa_family == AF_INET) 
		return &(((struct sockaddr_in*)sa)->sin_addr);
    //IPv6
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;
    fd_set read_fds;  

    int fdmax;        
    // listening socket descriptor
    int listener;
    // newly accept()ed socket descriptor
    int newfd;
    // client address
    struct sockaddr_storage remoteaddr; 
    socklen_t addrlen;
	struct addrinfo hints, *ai, *p;

    // buffer for client data
    char buf[BUFSIZE];    
    int nbytes;

//?
	char remoteIP[INET6_ADDRSTRLEN];

    // for setsockopt() SO_REUSEADDR, below
    int yes=1;        
    int i, j, rv;


    // clear the master and temp sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET; //use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //fill in my ip for me

    
    /*The getaddrinfo() function allocates and initializes a linked list of addrinfo structures,
    one for each network address that matches node and service, subject to any restrictions imposed by hints,
     and returns a pointer to the start of the list in res.*/

    //The gai_strerror() function shall return a text string describing an error value for the getaddrinfo()
	if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
    //loop po file_descriptor i na 1 stawiamy socket
	for(p = ai; p != NULL; p = p->ai_next){
        //create socket
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(listener < 0)
			continue;
		
		//lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        //bind
		if(bind(listener, p->ai_addr, p->ai_addrlen) < 0){
			close(listener);
			continue;
		}

		break;
	}

	//if we got here, it means we didn't get bound
	if(p == NULL){
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

    //free linked list
	freeaddrinfo(ai); 

    //listen
    if(listen(listener, 10) == -1){
        perror("listen");
        exit(3);
    }

    //add the listener to the master set
    FD_SET(listener, &master);

    //keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    //main loop
    while(1){
        read_fds = master; //kopia, bo select zmiena deskryptory
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(4);
        }

        for(i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == listener){

                    //handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);

					if (newfd == -1){
                        perror("accept");
                    }

                    else{
                        FD_SET(newfd, &master); 
                        if(newfd > fdmax)
                            fdmax = newfd;
                         printf("[+]Connection from \n");
                    }
                }
                
                //handle data from a client
                else {
                    //diconect lub error
                    if((nbytes = recv(i, buf, sizeof buf, 0)) <= 0){
                        if (nbytes == 0) 
                            printf("[+]Client disconected\n");
                        else 
                            perror("recv");
                        
                        close(i);
                        FD_CLR(i, &master);
                    }


                    else{
                        buf[nbytes] = '\0';

                        char bufcpy[BUFSIZE];
                        strncpy(bufcpy,buf,BUFSIZE); 
                        printf("%s\n",bufcpy);
                        
                        if(buf[0] == '/'){
                            //wysylanie pliku
                            printf("wysylanie pliku TODO");
                        }

                        //wysylanie do wszytkich z setu poza listenerem i samymi nami, cos sie gubi
                        //************************************************************************//
                        // else{
                        //     for(j = 0; j <= fdmax; j++){
                        //         if(FD_ISSET(j, &master)){
                        //             if(j != listener && j != i) {
                        //                 if(send(j, buf, nbytes, 0) == -1){
                        //                     perror("send");
                        //                 }
                        //             }
                        //         }
                        //     }
                        // }                        
                    }
                }
            } 
        } 
    } 
    return 0;
}