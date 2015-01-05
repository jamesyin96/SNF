serverSNFS.c
This is the server side program. It contains the server side API
s_open(),s_close(),s_read(),s_write() and s_stat()
To use the server progoram, one should type:
gcc -g -o serverSNFS serverSNFS.c
To run any program below, you should always run serverSNFS first.
The sample running command is below:
./serverSNFS --port 16892 --mount /tmp/yy291


clientSNFS.c
This is the client API library, the declarations of all the API are in client_api.h
the main functions are:
void setServer(char* server,int portnum)
int openFile(char* name);
int closeFile(int fd);
int readFile(int fd, void* buf);
int writeFile(int fd, void* buf);
int statFile(int fd, struct fileStat *buf);

To use this API library, for example, your program clientapp.c needs to use it. First run serverSNFS, second open another terminal. Also clientapp.c should include "client_api.h". The compilation should be:
gcc -g -o clientapp clientapp.c clientSNFS.c


clientapp.c
This is the clientapp program described in the assignment description. To run this clientapp program,
one should compile it with the clientSNFS.c in a different terminal window than the serverSNFS. To compile the program, you should type:
gcc -g -o clientapp clientapp.c clientSNFS.c


loadtes.c
This is the program for testing the throughput of the server. The compiling process is like clientapp.c. After running the program, you will see the throughput for each client process, then you can add them together to get the total throughpu for the server. You can change the source code to measure the throughput with different number of client processes.


test.c
This is the general test program for testing the clientSNFS API. To run this test program, the comopiling process is like clientapp.c