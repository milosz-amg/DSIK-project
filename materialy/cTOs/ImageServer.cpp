#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<fstream>
#include<errno.h>

using namespace std;

int receive_image(int socket)
{ // Start function

int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

char imagearray[10241],verify = '1';
FILE *image;

//Find the size of the image
do{
stat = read(socket, &size, sizeof(int));
}while(stat<0);

/*printf("Packet received.\n");
printf("Packet size: %i\n",stat);
printf("Image size: %i\n",size);
printf(" \n");*/

char buffer[] = "Got it";

//Send our verification signal
do{
stat = write(socket, &buffer, sizeof(int));
}while(stat<0);

printf("Reply sent\n");
printf(" \n");

image = fopen("res.ppm", "w");

if( image == NULL) {
printf("Error has occurred. Image file could not be opened\n");
return -1; }

//Loop while we have not received the entire file yet


int need_exit = 0;
struct timeval timeout = {10,0};

fd_set fds;
int buffer_fd, buffer_out;

while(recv_size < size) {
//while(packet_index < 2){

    FD_ZERO(&fds);
    FD_SET(socket,&fds);

    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

    if (buffer_fd < 0)
       printf("error: bad file descriptor set.\n");

    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");

    if (buffer_fd > 0)
    {
        do{
               read_size = read(socket,imagearray, 10241);
            }while(read_size <0);

            /*printf("Packet number received: %i\n",packet_index);
            printf("Packet size: %i\n",read_size);*/


        //Write the currently read data into our image file
         write_size = fwrite(imagearray,1,read_size, image);
         //printf("Written image size: %i\n",write_size);

             if(read_size !=write_size) {
                 printf("error in read write\n");    }


             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             /*printf("Total received image size: %i\n",recv_size);
             printf(" \n");
             printf(" \n");*/
    }

}


  fclose(image);
  printf("Image successfully Received!\n");
  return 1;
  }


int main(int argc , char *argv[])
{
    int socket_desc , new_socket , c, read_size,buffer = 0;
    struct sockaddr_in server , client;
    char *readin;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
      printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8889 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
      puts("bind failed");
      return 1;
    }

    puts("bind done");
    //Listen
    listen(socket_desc , 3); // int listen(int s, int backlog); | `backlog` (3)
    //limits the number of outstanding connections in the socket's listen
    //queue to the value specified by the backlog argument.

    while(true)
    {
      //Accept and incoming connection
      puts("Waiting for incoming connections...");
      c = sizeof(struct sockaddr_in);
      // waits for a connection. returns -1 on failure, a positive val on success
      if((new_socket = accept(socket_desc, (struct sockaddr *)&client,(socklen_t*)&c)))
      {
          puts("Connection accepted");
          cout << "new_socket = " << new_socket << "\n";
      }
      fflush(stdout);
      if (new_socket<0)
      {
          perror("Accept Failed, trying again");
          continue;
      }

      // receive image
      receive_image(new_socket);
    }

    close(socket_desc);
    fflush(stdout);
    return 0;
}