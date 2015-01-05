#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "client_api.h"

// a test client app
int main (){

  setServer("cd.cs.rutgers.edu",16892);

  char readbuf[1024];
  int read1 = readFile(10, readbuf);
  printf("read1 is: %d\n", read1);

  int desc = openFile("hello.in");
  printf("open file1, fd is: %d\n", desc);
  
  
  char read_buf[1024]="";
  
  char write_buf[] = "this is the second test.";
  int write_return = writeFile(desc,write_buf);
  printf("the write result is: %d\n", write_return);
  
  struct fileStat *statbuf1;
  statbuf1 = (struct fileStat *) malloc (sizeof(struct fileStat));
  statFile(desc,statbuf1);
  printf("File status is: size: %d\tcreate time: %s\taccess time: %s\tmodification time: %s\n", statbuf1->fsize, statbuf1->c_time, statbuf1->a_time, statbuf1->m_time);
 
  int read = readFile(desc,read_buf);
  printf("the read result is: %d\n", read);
  printf("the read content is: %s, content size is: %lu\n", read_buf, strlen(read_buf));

  int desc2 = openFile("hello.out");
  printf("the new fd is: %d\n", desc2);

  int write_return2 = writeFile(desc2,read_buf);
  printf("the new write result is: %d\n", write_return2);

  char read_buf2[1024]="";
  int read_return = readFile(desc2, read_buf2);
  printf("read the new writing: %s\n", read_buf2);

  struct fileStat *statbuf2;
  statbuf2 = (struct fileStat *) malloc (sizeof(struct fileStat));
  statFile(desc2,statbuf2);
  printf("File status is: size: %d\tcreate time: %s\taccess time: %s\tmodification time: %s\n", statbuf2->fsize, statbuf2->c_time, statbuf2->a_time, statbuf2->m_time);
  
  int close = closeFile(desc);
  printf("close file1, 0 if successful: %d\n", close);

  int close3 = closeFile(desc);
  printf("close file1, 0 if successful: %d\n", close3); 
  
  int close2 = closeFile(8);
  printf("close file2, 0 if success: %d\n", close2);
  
  free(statbuf1);
  free(statbuf2);

  return 0;
}
