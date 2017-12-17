#ifndef FORWARDINGTABLE_H_
#define FORWARDINGTABLE_H_

#include "../include/packetlib.h"
#include "../include/router.h"


#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include <vector>
#include <stack>
#include <deque>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string>

std::vector<table_entry> createForwardingTable(std::vector<int> dv);
std::vector<table_entry> updateForwardingTable(std::map<int,neighbor>,int numrouters,int srcid,std::vector<int> self_dv);



#endif

