#ifndef NETWORK_H_
#define NETWORK_H_

int sendAll(int sock,char *buffer,int numBytes);
int recvAll(int sock,char *buffer,int numBytes);

#endif