#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "client_api.h"
#define max_buf 1024

struct sockaddr_in peerAddress, ownAddress;
int sessionSocket;

// global variable for serverIP and port
char* serverIP;
int port;

// instruction identification list

// openFile  request_type = 1
// closeFile request_type = 2
// readFile  request_type = 3
// writeFile request_type = 4
// statFile  request_type = 5
char request_type;

char* host2IpAddr(char *anIpName){
  /* input   : hostname in ip format such as cs.rutgers.edu
  * returns : 0 if any error ipaddr otherwise
  * output  : -
  * desc    : resolves an ip name into dot-number IP address.
  */
  struct hostent *hostEntry;
  struct in_addr *scratch;
 
  if ((hostEntry = gethostbyname ( anIpName )) == (struct hostent*) NULL)
    return 0;
 
  scratch = (struct in_addr *) hostEntry->h_addr;
  return (inet_ntoa(*scratch));
}

void setServer(char* server,int portnum){
  serverIP = server;
  port = portnum;
  //determine if the input serverIP is a hostname, if so, make it a host IP
  if (atoi(server) == 0){
  serverIP = host2IpAddr(server);
  }
  //printf("serverIP is: %s\n", serverIP);
}

int openFile(char* name){
  char buf_msg[32]={""};
  //printf("\nthis is openFile\n");
  // organize all send information
  request_type = '1';
  buf_msg[0] = '1';
  strcat(buf_msg, name);
  
  // receive information
  char fd_return[32];
  int fd_return_value;

  memset ((void*)&peerAddress,0,sizeof(peerAddress));
  memset ((void*)&ownAddress,0,sizeof(ownAddress));
  if((sessionSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  //printf("print out the session socket: %d\n", sessionSocket);
  peerAddress.sin_addr.s_addr = inet_addr (serverIP);
  peerAddress.sin_port = htons(port);
  //printf("port number: %d\n", port);
  peerAddress.sin_family = AF_INET;

  if(connect (sessionSocket, (struct sockaddr *)&peerAddress, sizeof (peerAddress)) < 0){
    printf("Unable to connect to the server.%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  
  //printf("The sending message is: %s\n", buf_msg);
  
  write (sessionSocket, buf_msg, 32);

  read (sessionSocket, fd_return, 32);

  close (sessionSocket);
  
  fd_return_value = atoi(fd_return);
  return fd_return_value;
}

int closeFile(int fd){
  char buf_msg[32]={""};
  char s_fd[31];
  sprintf(s_fd, "%d", fd);
  //printf("\nthis is closeFile\n");
  // organize all send information
  request_type = '2';
  buf_msg[0] = '2';
  strcat(buf_msg, s_fd);
  
  // receive information
  char close_return[32];
  int close_return_value;
  memset ((void*)&peerAddress,0,sizeof(peerAddress));
  memset ((void*)&ownAddress,0,sizeof(ownAddress));
  if((sessionSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  //printf("print out the session socket: %d\n", sessionSocket);
  peerAddress.sin_addr.s_addr = inet_addr (serverIP);
  peerAddress.sin_port = htons(port);
  peerAddress.sin_family = AF_INET;

  if(connect (sessionSocket, (struct sockaddr *)&peerAddress, sizeof (peerAddress)) < 0){
    printf("Unable to connect to the server.%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  
  //printf("The sending message is: %s\n", buf_msg);
  
  write (sessionSocket, buf_msg, 32);

  read (sessionSocket, close_return, 32);
  //printf("receive close_return: %s\n", close_return);
  close (sessionSocket);
  close_return_value = atoi(close_return);
  return close_return_value;
}

int readFile(int fd, void* buf){
  char buf_msg[32]={""};
  char recv_msg1[32]="";
  char recv_msg2[1024]="";
  char s_fd[31];
  sprintf(s_fd, "%d", fd);
  //printf("\nthis is readFile\n");
  //printf("input fd is: %s\n", s_fd);
  // organize all send information
  request_type = '3';
  //printf("buf_msg was: %s\n", buf_msg);
  buf_msg[0] = '3';
  strcat(buf_msg, s_fd);
  
  //printf("buf_msg is: %s\n", buf_msg);
  // receive information
  int read_return = 0;

  memset ((void*)&peerAddress,0,sizeof(peerAddress));
  memset ((void*)&ownAddress,0,sizeof(ownAddress));
  if((sessionSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  //printf("print out the session socket: %d\n", sessionSocket);
  peerAddress.sin_addr.s_addr = inet_addr (serverIP);
  //printf("server IP: %s\n", serverIP);
  
  peerAddress.sin_port = htons(port);
  //printf("port number: %d\n", port);
  peerAddress.sin_family = AF_INET;

  if(connect (sessionSocket, (struct sockaddr *)&peerAddress, sizeof (peerAddress)) < 0){
    printf("Unable to connect to the server.%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  
  //printf("The sending message is: %s\n", buf_msg);
  
  write (sessionSocket, buf_msg, 32);

  read (sessionSocket, recv_msg1, 32);
  read (sessionSocket, recv_msg2, 1024);

  read_return = atoi(recv_msg1);
  
  if (read_return != -1)
  {
    strncpy(buf, recv_msg2, read_return);
  }
  //printf("strncpy size is: %lu, %d\n,", strlen(buf),read_return);
  close (sessionSocket);

  return read_return;
}

int writeFile(int fd, void* buf){
  char buf_msg[32]={""};
  char s_fd[31];
  char buf_size[8];
  sprintf(s_fd, "%d", fd);
  //printf("\nthis is writeFile\n");
  
  // organize all send information
  request_type = '4';
  buf_msg[0] = '4';
  strcat(buf_msg, s_fd);
  
  //count the number of bytes of the buf
  int nbytes = strlen(buf);
  sprintf(buf_size, "%d", nbytes);

  //printf("buf_msg is: %s\n", buf_msg);
  // receive information
  int write_return = -1;
  char return_buf[32]={""};
  memset ((void*)&peerAddress,0,sizeof(peerAddress));
  memset ((void*)&ownAddress,0,sizeof(ownAddress));
  if((sessionSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  //printf("print out the session socket: %d\n", sessionSocket);
  peerAddress.sin_addr.s_addr = inet_addr (serverIP);
  //printf("server IP: %s\n", serverIP);
  
  peerAddress.sin_port = htons(port);
  peerAddress.sin_family = AF_INET;

  if(connect (sessionSocket, (struct sockaddr *)&peerAddress, sizeof (peerAddress)) < 0){
    printf("Unable to connect to the server.%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  
  //printf("The sending message is: %s\n", buf_msg);
  
  write (sessionSocket, buf_msg, 32);
  write (sessionSocket, buf_size, 8);
  //printf("the write info is: %s\n", buf);
  write (sessionSocket, buf, 1024);

  read (sessionSocket, return_buf, 32);

  write_return = atoi(return_buf);

  close (sessionSocket);

  return write_return;
}

int statFile(int fd, struct fileStat *buf){
  char buf_msg[32]={""};
  char recv_msg1[1024]={""};
  char recv_msg2[1024]={""};
  char s_fd[31];
  //printf("\nthis is statFile\n");

  sprintf(s_fd, "%d", fd);
  request_type = '5';
  buf_msg[0] = '5';
  strcat(buf_msg, s_fd);
  //printf("buf_msg is: %s\n", buf_msg);
  
  // set initial information return value
  int stat_return = 0;

  memset ((void*)&peerAddress,0,sizeof(peerAddress));
  memset ((void*)&ownAddress,0,sizeof(ownAddress));
  if((sessionSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  //printf("print out the session socket: %d\n", sessionSocket);
  peerAddress.sin_addr.s_addr = inet_addr (serverIP);
  //printf("server IP: %s\n", serverIP);
  
  peerAddress.sin_port = htons(port);
  //printf("port number: %d\n", port);
  peerAddress.sin_family = AF_INET;

  if(connect (sessionSocket, (struct sockaddr *)&peerAddress, sizeof (peerAddress)) < 0){
    printf("Unable to connect to the server.%d\n",errno);
    perror("meaning:"); exit(0);
    return -1;
  }
  
  //printf("The sending message is: %s\n", buf_msg);
  
  write (sessionSocket, buf_msg, 32);

  read (sessionSocket, recv_msg1, 32);
  read (sessionSocket, recv_msg2, 1024);

  stat_return = atoi(recv_msg1);
  
  //printf("recieve stat message is: %s\n", recv_msg2); 

  //now parse the information from recv_msg2 and store it to the buf
  char *ptr = recv_msg2;
  int i = 0;
  int n;
  char field[4][32];
  while ( sscanf(ptr, "%31[^;]%n", field[i], &n) == 1 )
  {
    ptr += n; /* advance the pointer by the number of characters read */
    if ( *ptr != ';' )
    {
      break; /* didn't find an expected delimiter, done? */
    }
    ++ptr; /* skip the delimiter */
    ++i;
  }
  buf->fsize = atoi(field[0]); //file size
  strcpy(buf->c_time, field[1]);
  strcpy(buf->a_time, field[2]);
  strcpy(buf->m_time, field[3]);
 


  return stat_return;
}