#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "client_api.h"
#include <time.h>
//#define SERVER_PORT 16892
struct sockaddr_in peerAddress, ownAddress;
int serverSocket, sessionSocket, size;
char request_type = '0';
char* WORKDIR;
char filepath[128];
char buff[1024];



//** struct and its function to store the file name, file descriptor and other information **//
int current_pos = 0;

struct fdtableentry {
  int fd;
  char filename[32];
  int opencount;
  int fsize;
  time_t c_time;
  time_t a_time;
  time_t m_time;
}fdtable[1024];

/*check if the filename exist in the fdtable, if so return fd, otherwise return -1 */
int checkexist(char *filename){
  int i = 0;
  int result = -1;
  for (i = 0; i < current_pos; ++i)
  {
    if (strcmp(fdtable[i].filename, filename) == 0){
      result = fdtable[i].fd;
      break;
    }
  }
  return result;
}

int addfd(char* filename, int fd){
  time_t current_time;
  current_time = time(NULL);
  strcpy(fdtable[current_pos].filename, filename);
  fdtable[current_pos].fd = fd;
  fdtable[current_pos].opencount = 1;
  fdtable[current_pos].c_time = current_time;
  fdtable[current_pos].a_time = current_time;
  fdtable[current_pos].m_time = current_time;
  fdtable[current_pos].fsize = 0;
  current_pos++;
  return current_pos;
}

int findfd(int fd){
  int i = 0;
  int result = -1;
  for (i = 0; i < current_pos; ++i)
  {
    if (fdtable[i].fd == fd){
      result = i;
      break;
    }
  }
  return result;
}

//** main server side API **//

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

void redirect(char* dir, char* name){
  char result[128];
  //in real code you would check for errors in malloc here
  strcpy(result, dir);
  strcat(result, "/");
  strcat(result, name);
  strncpy(filepath,result,128);
}

// perform required execution for command "open"
void s_open(char *name){
  int fd;
  char fd_return[32]="";
  //printf("\nthis is s_open\n");
  redirect(WORKDIR, name);
  printf("the file path is: %s\n", filepath);
  // open the file
  int checkfd = checkexist(name);
  if (checkfd == -1)
  {
    fd = open(filepath, O_RDWR | O_CREAT, 0777);
    if(fd == -1) {
      printf("Something went wrong with openFile()! %s\n", strerror(errno));
    }
    int change = fchmod(fd, 0777);
    addfd(name, fd);
  }else{
    fd = checkfd;
    int fdlocate = findfd(fd);
    fdtable[fdlocate].opencount++; 
  }
  sprintf(fd_return, "%d", fd);
  //printf("file descriptor is: %s\n", fd_return);
  // send the return to the client
  write(sessionSocket, fd_return , 32);
  //printf("s_open ends\n");
}

void s_close(int fd){
  //printf("\nthis is s_close\n");
  // close the file
  int close_fd;
  char close_return[32];
  int fdloc = findfd(fd);
  //printf("find fd: %d\n", fdloc);
  if (fdloc == -1){
    close_fd = -1;
  }else if(fdtable[fdloc].opencount <= 0){
    close_fd = -1; 
  }else if (fdtable[fdloc].opencount > 1)
  {
    close_fd = 0;
    fdtable[fdloc].opencount--;
  }else{
    //printf("really close once.\n");
    //printf("close fd is: %d\n", fd);
    close_fd = close(fd);
    //printf("really close return: %d\n", fcntl(fd, F_GETFD));
    if(close_fd == -1) {
      printf("Something went wrong with closeFile()! %s\n", strerror(errno));
    }
    fdtable[fdloc].opencount--;  
  }
  sprintf(close_return, "%d", close_fd);
  // send the return to the client
  write(sessionSocket, close_return, 32);
  //printf("s_close ends\n");
}

void s_read(int fd){
  //printf("\nthis is s_read\n");
  char read_buf[1024]={""};
  char return_msg[32]={""};
  int file_size;
  int read_return;
  struct stat buf;
  int checkfd = findfd(fd);
  if (checkfd == -1){
    read_return = -1;
  }else if(fdtable[checkfd].opencount<=0){
    read_return = -1;
  }else{
    fstat(fd, &buf);
    file_size = buf.st_size;
    //printf("file size is: %d\n", file_size);
    // open the file
    read_return = read(fd, read_buf, file_size);
    if(read_return == -1) {
    printf("Something went wrong with readFile()! %s\n", strerror(errno));
    }
    lseek(fd, 0, SEEK_SET); 
    fdtable[checkfd].a_time = time(NULL);
  }
  //combine the read return info and the read buf into one sedning message
  sprintf(return_msg, "%d", read_return);
  write(sessionSocket, return_msg, 32);
  // send the return to the client
  //printf("the sending message is: %s\n", read_buf);
  write(sessionSocket, read_buf, 1024);
  //printf("s_read end.\n");
  //printf("end of s_read.\n");
}

void s_write(int fd, char* write_buf, int length){
  //printf("\nthis is s_write\n");
  char return_msg[32]="";
  int write_return;
  int checkfd = findfd(fd);
  //printf("open count: %d\n", fdtable[checkfd].opencount);
  if (checkfd == -1)
  {
    write_return = -1;
  }else if ((fdtable[checkfd].opencount <= 0)){
    write_return = -1;
  }else{
    // resize the target file to a size of length bytes.
    int cut = ftruncate(fd, length);
    //printf("the write file size is: %d\n", length);
    //printf("cut result is: %d, cut bytes: %d \n", cut, length);
    //printf("the writing content is: %s\n", write_buf);
    write_return = write(fd, write_buf, length);
    if(write_return == -1) {
      printf("Something went wrong with readFile()! %s\n", strerror(errno));
    }
    lseek(fd, 0, SEEK_SET);
    fdtable[checkfd].a_time = time(NULL);
    fdtable[checkfd].m_time = time(NULL);
    fdtable[checkfd].fsize = length;
  }
  //combine the read return info and the read buf into one sedning message
  sprintf(return_msg, "%d", write_return);
  
  //printf("the sending message is: %s\n", return_msg);
  // send the return to the client
  write(sessionSocket, return_msg, 32);
  //printf("s_write ends\n");
}

void s_stat(int fd){
  printf("\nthis is s_stat\n");
  char stat_info[1024]={""};
  char return_msg[32]={""};
  char str_fsize[32]={""};
  int stat_return;
  int f_size;  //file size
  time_t c_time;   //file creation time
  time_t a_time; //file last access time
  time_t m_time; //file last modification time
  struct stat *buf;
  buf = (struct stat *) malloc (sizeof(struct stat));
  int checkexist = findfd(fd);
  if (checkexist == -1)
  {
    stat_return = -1;
  }else if (fdtable[checkexist].opencount <= 0){
    stat_return = -1;
  }else{
    // first get the file size info, create time, modification time and last access time
    f_size = fdtable[checkexist].fsize;
    c_time = fdtable[checkexist].c_time;
    a_time = fdtable[checkexist].a_time;
    m_time = fdtable[checkexist].m_time;
    
    // put the infoamtion into a string
    sprintf(str_fsize, "%d", f_size);
    strcat(stat_info, str_fsize);
    strcat(stat_info, ";");
    strcat(stat_info, ctime(&c_time));
    strcat(stat_info, ";");
    strcat(stat_info, ctime(&a_time));
    strcat(stat_info, ";");
    strcat(stat_info, ctime(&m_time));
    stat_return = 0;
  }
  
  if(stat_return == -1) {
    printf("Something went wrong with statFile()!\n");
  }
  
  //combine the read return info and the read buf into one sedning message
  sprintf(return_msg, "%d", stat_return);
  write(sessionSocket, return_msg, 32);
  //printf("the sending message is: %s\n", stat_info);
  // send the return to the client
  write(sessionSocket, stat_info, 1024);
  //printf("s_stat end.\n");
  //printf("end of s_stat.\n");
  free(buf);
}


int main (int argc, char *argv[]){

  
  //char data[256] = "Hi!!!----from Server\n";
  char hostname[32]={"localhost"};
  printf("hostname is: %s\n",hostname);
  int SERVER_PORT=16892;

  if (argc < 5){
    printf("Usage: nfs_server [OPTION]\n");
    printf("Server options:\n");
    printf("  -p, --port=PORT   Listen on this port\n");
    printf("  -m, --mount= Working Directory of the Server\n");
    exit(1);
  }

  //printf("input variables number correct.\n");
  // parse the command line argument
  static struct option long_options[] =
    {
      {"port",  required_argument, 0, 'p'},
      {"mount",  required_argument, 0, 'm'},
      {0, 0, 0, 0}
    };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  int c = 0;
  // start parsing the option arguments
  while ((c = getopt_long (argc, argv, "p:m:",
          long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'p':
        SERVER_PORT = atoi(optarg);
        break;

      case 'm':
        WORKDIR = optarg;
        break; 

      case '?':
        /* getopt_long already printed an error message. */
        printf("Usage: nfs_server [OPTION]\n");
        printf("Server options:\n");
        printf("  -p, --port=PORT   Listen on this port\n");
        printf("  -m, --mount= Working Directory of the Server\n");
        exit(1);

      default:
        printf("Usage: nfs_server [OPTION]\n");
        printf("Server options:\n");
        printf("  -p, --port=PORT   Listen on this port\n");
        printf("  -m, --mount= Working Directory of the Server\n");
        exit(EXIT_FAILURE);
    }
  }

  printf("working directory is: %s\n", WORKDIR);
  int status;
  status = mkdir(WORKDIR, 0755);
  //printf("create working directory: 0 is successful! %d\n", status);

  memset ((void*)&peerAddress,0,sizeof(peerAddress));
  memset ((void*)&ownAddress,0,sizeof(ownAddress));
 
  size = sizeof(peerAddress);

  serverSocket = socket (AF_INET, SOCK_STREAM, 0);
  printf("show serversocket: %d\n", serverSocket);
  gethostname (hostname, 32);

  ownAddress.sin_port = htons(SERVER_PORT);
  printf("print port number: %d\n", SERVER_PORT);
  ownAddress.sin_family = AF_INET;
  //ownAddress.sin_addr.s_addr = INADDR_ANY;
  ownAddress.sin_addr.s_addr = inet_addr(host2IpAddr (hostname));
  printf("Server own IP: %s\n", host2IpAddr(hostname));
  bind (serverSocket, (struct sockaddr *)&ownAddress, sizeof(ownAddress));
	  
  int checklisten = listen (serverSocket, 128);
  printf("check listen status: %d\n", checklisten);

  while(1){
    sessionSocket = accept (serverSocket, (struct sockaddr *)&peerAddress, &size);

    //printf("\nclient IP: %u", peerAddress.sin_addr.s_addr);
    if (sessionSocket  < 0) {
        fprintf(stderr, "Error: client - accept failed, error %d\n",errno);
        perror("Error means:");
        exit(1);
    }

    request_type = '0';
    read(sessionSocket, &request_type, 1);

    // check the first 3 bytes to determine request type
    // type 1, openFile
    if (request_type == '1'){
      char open_filename[32];
      //printf("\nreceive open request.\n");
      read(sessionSocket, open_filename, 31);
      //printf("file name received is: %s\n", open_filename);
      //printf("process open request.\n");
      s_open(open_filename);
      //printf("finish open request.\n");
    }else if (request_type == '2'){
      char recv_fd[32];
      //printf("\nreceive close request.\n");
      read(sessionSocket, recv_fd, 31);
      //printf("file descriptor received is: %s\n", recv_fd);
      int close_fd = atoi(recv_fd);
      //printf("process close request\n");
      s_close(close_fd);
      //printf("finish close request\n");
      
    }else if (request_type == '3'){
      char recv_fd[32];
      //printf("\nrecieve read request.\n");
      read(sessionSocket, recv_fd, 31);
      //printf("file descriptor received is: %s\n", recv_fd);
      int read_fd = atoi(recv_fd);
      //printf("process read request\n");
      s_read(read_fd);
      //printf("finish read request\n");
    }else if (request_type == '4'){
      char recv_fd[32];
      char write_buf[1024];
      char write_length_char[8];
      int write_length;
      //printf("\nreceive write request.\n");
      read(sessionSocket, recv_fd, 31);
      read(sessionSocket, write_length_char, 8);
      read(sessionSocket, write_buf, 1024);
      //printf("file descriptor received is: %s\n", recv_fd);
      int read_fd = atoi(recv_fd);
      write_length = atoi(write_length_char);
      //printf("process write request\n");
      s_write(read_fd, write_buf, write_length);
      //printf("finish write request\n");
    }else if(request_type == '5'){
      char recv_fd[32];
      //printf("\nreceive statFile request.\n");
      read(sessionSocket, recv_fd, 31);
      //printf("file descriptor received is: %s\n", recv_fd);
      int stat_fd = atoi(recv_fd);
      //printf("process stat request\n");
      s_stat(stat_fd);
      //printf("finish stat request\n");
    }

  }

  close (sessionSocket);

  close (serverSocket);

  return 0;

}
