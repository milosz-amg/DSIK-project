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

void FileReciving(){
    printf("wysylanie pliku TODO\n");
}

int main(void)
{
    fd_set master;
    fd_set read_fds;  

    int fdmax;        
    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr; 
    socklen_t addrlen;
	struct addrinfo hints, *ai, *p;

    char buf[BUFSIZE];    
    int nbytes;

    int yes=1;        

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    
    
	if((getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
		printf("linked list creation error\n");
		return 1;
	}
	
    //loop po file_descriptor i na 1 stawiamy socket
	for(p = ai; p != NULL; p = p->ai_next){

		if((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol))< 0){
			continue;
        }		
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if(bind(listener, p->ai_addr, p->ai_addrlen) < 0){
			close(listener);
			continue;
		}
		break;
	}

	if(p == NULL){
		printf("Bind error\n");
		return 1;
	}

	freeaddrinfo(ai); 

    if(listen(listener, 10) == -1){
        perror("listen");
        return 1;
    }
    FD_SET(listener, &master);
    fdmax = listener; 



    while(1){
        read_fds = master; //kopia, bo select zmiena deskryptory
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            printf("select");
            return 1;
        }

        for(int i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == listener){
                    //handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
					if (newfd == -1){
                        printf("accept error");
                    }
                    else{
                        FD_SET(newfd, &master); 
                        if(newfd > fdmax)
                            fdmax = newfd;
                         printf("[+]New connection\n");
                    }
                }

                //Data
                else {
                    //diconect or error
                    if((nbytes = recv(i, buf, sizeof buf, 0)) <= 0){
                        if (nbytes == 0) 
                            printf("[+]Client disconected\n");
                        else 
                            printf("recv error\n");
                        
                        close(i);
                        FD_CLR(i, &master);
                    }
                    //message or file
                    else{
                        buf[nbytes] = '\0';

                        if(buf[0]=='<' && buf[1]=='<'){
                            FileReciving();
                        }
                        else{
                            char bufcpy[BUFSIZE];
                            strncpy(bufcpy,buf,BUFSIZE); 
                            printf("%s\n",bufcpy);
                        }
                        //wysylanie do wszytkich z setu poza listenerem i samymi nami, cos sie gubi
                        //************************************************************************//
                        // else{
                        //     for(int j = 0; j <= fdmax; j++){
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