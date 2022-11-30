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

#define PORT 7777
#define BUFSIZE 512

int sendFile(int reciever, char name[]){
    printf("Preparing %s to delivery\n",name);
    FILE *file;
    int size=0, read_size=0, now_send=0, delivered=0, packet_size=0;
    char buf[BUFSIZE];
    char chsize[BUFSIZE];
    memset(buf,0,sizeof buf);
    memset(chsize,0,sizeof chsize);

    file=fopen(name, "rb");
    if(file == NULL){
        printf("file opening error");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size=ftell(file);
    fseek(file,0,SEEK_SET);

    printf("File size: %i\n",size);
    sprintf(chsize,"%d",size);
    

    send(reciever,chsize,sizeof chsize,0); //wysylanie wielkosci pliku

    while(delivered<size){
        packet_size=fread(buf,1,BUFSIZE,file);
        if((now_send = send(reciever, buf, packet_size,0))!=packet_size){
            break;
        }
        delivered=delivered+now_send;
        printf("now sent: %i\n", now_send);
        printf("delivered in total: %i\n",delivered);
    }

    fclose(file);
    printf("File %s delivered\n",name);
    return 0;
}

int main(void)
{

    int socket_server;
    int newfd;
    int fdmax;        
    char buf[BUFSIZE];    
    int nbytes;
    int yes=1;  
    
    fd_set master;
    fd_set read_fds;  
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    struct sockaddr_in myaddr;
    myaddr.sin_family=AF_INET;
    myaddr.sin_port=htons(PORT);
    myaddr.sin_addr.s_addr=INADDR_ANY;
    
    socket_server=socket(PF_INET, SOCK_STREAM,0);   //if socket error

    if(bind(socket_server,(struct sockaddr*) &myaddr, sizeof(struct sockaddr_in))<0){
        printf("bind error");
        return 1;
    }

    FD_SET(socket_server, &master);
    fdmax = socket_server; 
    
    if(listen(socket_server, 10) == -1){
        perror("listen error");
        return 1;
    }

    while(1){
        read_fds = master; //kopia, bo select zmiena deskryptory
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) < 0 ){
            printf("select error");
            return 1;
        }
        for(int i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == socket_server){
                    //handle new connections
                    socklen_t adrlen = sizeof(struct sockaddr_in);
					newfd = accept(socket_server,(struct sockaddr *) &myaddr, &adrlen);
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

                        char bufcpy[BUFSIZE];
                        strncpy(bufcpy,buf,BUFSIZE); 
                        printf("%s\n",bufcpy);

                        if(buf[0]=='>' && buf[1]=='>'){
                            //sent to i
                            char filename[BUFSIZE-2];
                            strncpy(filename,buf+2,BUFSIZE-2);
                            sendFile(i,filename);

                        }
                        //wysylanie do wszytkich z setu poza listenerem i samymi nami, cos sie gubi
                        //************************************************************************//
                        // else{
                        //     for(int j = 0; j <= fdmax; j++){
                        //         if(FD_ISSET(j, &master)){
                        //             if(j != socket_server && j != i) {
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