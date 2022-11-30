#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

void DrukujNadawce(struct sockaddr_in *adres)
{
  printf("Wiadomosc od %s:%d",
    inet_ntoa(adres->sin_addr),
    ntohs(adres->sin_port)
    );  
}

void ObsluzTCP(int gniazdo, struct sockaddr_in *adres)
{
  int nowe_gniazdo;
  char bufor[1024];
  socklen_t dladr = sizeof(struct sockaddr_in);
  nowe_gniazdo = 
    accept(gniazdo, (struct sockaddr*) adres, &dladr);
  if (nowe_gniazdo < 0)
  {
    printf("Bledne polaczenie (accept < 0)\n");
    return;
  }
  memset(bufor, 0, 1024);
  while (recv(nowe_gniazdo, bufor, 1024, 0) <= 0);
  DrukujNadawce(adres);
  printf("[TCP]: %s\n", bufor);
  close(nowe_gniazdo);
}


void ObsluzObaProtokoly(int gniazdoTCP, int gniazdoUDP, struct sockaddr_in *adres)
{
  fd_set readfds;
  struct timeval timeout;
  unsigned long proba;
  int maxgniazdo;
  
  maxgniazdo = (gniazdoTCP > gniazdoUDP ?
    gniazdoTCP+1 : gniazdoUDP+1);
  proba = 0;

  while(1)
  {
    FD_ZERO(&readfds);
    FD_SET(gniazdoTCP, &readfds);
    FD_SET(gniazdoUDP, &readfds);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (select(maxgniazdo, &readfds, NULL, NULL, &timeout) > 0)
    {
      proba = 0;
      if (FD_ISSET(gniazdoTCP, &readfds))
        ObsluzTCP(gniazdoTCP, adres);
      if (FD_ISSET(gniazdoUDP, &readfds))
        ObsluzUDP(gniazdoUDP, adres);
    }
    else
    {
      proba++;
      printf("Czekam %lu sekund i nic ...\n", proba);
    }
  }
}

int main(void)
{
  struct sockaddr_in bind_me_here;
  int gt, gu, port;
  
  printf("Numer portu: ");
  scanf("%d", &port);
  
  gt = socket(PF_INET, SOCK_STREAM, 0);
  gu = socket(PF_INET, SOCK_DGRAM, 0);
  
  bind_me_here.sin_family = AF_INET;
  bind_me_here.sin_port = htons(port);
  bind_me_here.sin_addr.s_addr = INADDR_ANY;
  
  if (bind(gt,(struct sockaddr*) &bind_me_here,
           sizeof(struct sockaddr_in)) < 0)
  {
    printf("Bind na TCP nie powiodl sie.\n");
    return 1;
  }

  if (bind(gu,(struct sockaddr*) &bind_me_here,
           sizeof(struct sockaddr_in)) < 0)
  {
    printf("Bind na UDP nie powiodl sie.\n");
    return 1;
  }

  listen(gt, 10);
  
  ObsluzObaProtokoly(gt, gu, &bind_me_here);
  
  return 0;
}