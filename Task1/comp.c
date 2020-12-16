#include <stdio.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

# define BUFF_SIZE 2
// Shay Bratslavsky 313363541
// Omer Tati 305168049

int main(int argc,char** argv) {
    if (argc!=3){ // incase no 2 files given
        exit(1);
    }

    // assign files to variables
    char * firstFile = argv[1];
    char * secondFile = argv[2];

    // init buffers for files
    char* buf1 = calloc(BUFF_SIZE ,sizeof(char));
    char* buf2 = calloc(BUFF_SIZE, sizeof(char));

    // open 2 file discriptors
    int firstFd = open(firstFile, O_RDONLY);
    int secondFd = open(secondFile, O_RDONLY);

    // Make sure files can be open for reading
    if (firstFd == -1 || secondFd == -1){
        exit(1);
    }

    // Reading from files first buffer
    int read1 = read(firstFd, buf1,BUFF_SIZE-1);
    int read2 = read(secondFd, buf2,BUFF_SIZE-1);

    // while we are able to read we keep reading until end of file
    while(read1>0 && read2>0){
        if (buf1[0] != buf2[0]) // check if buffers read from files are the same
            exit(1);
        read1 = read(firstFd,buf1,BUFF_SIZE-1);
        read2 = read(secondFd,buf2,BUFF_SIZE-1);
    }

    if (read1!=0 || read2!=0) // if finished 1 file before the other
        exit(1);

    // clearing allocated buffers and closing files
    free(buf1);
    free(buf2);
    close(firstFd);
    close(secondFd);
    exit(2);
    return 0;
}