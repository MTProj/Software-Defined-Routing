#include "../include/network.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int sendAll(int sock,char *buffer,int numBytes){
    // Recv All
    int bytes = write(sock,buffer,numBytes);
    while(bytes != numBytes){
        bytes += write(sock,&buffer+bytes,numBytes - bytes);
    }
    return bytes;
}
int recvAll(int sock,char *buffer,int numBytes){
    // Recv All
    int bytes = read(sock,buffer,numBytes);
    if(bytes == 0){
        return 0;
    }
    while(bytes != numBytes){
        bytes += read(sock,&buffer+bytes,numBytes - bytes);
        if(bytes == 0){
            return 0;
        }
    }
    return bytes;
}
