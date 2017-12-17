#ifndef DISTANCEVECTOR_H_
#define DISTANCEVECTOR_H_

#include "../include/packetlib.h"
#include "../include/router.h"

#include <stdlib.h> // vector
#include <vector>
#include <iostream> //cout
#include <arpa/inet.h> //ntohs


class distancevector{
    public:
    distancevector();
    std::vector<int> createDistanceVector(int numrouters,std::vector<init_payload_router_entry*> routers);
    std::vector<int> updateDistanceVector(std::vector<table_entry> forwarding_table,std::vector<int> dv);
    private:

};

#endif