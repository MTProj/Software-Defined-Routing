#include "../include/distancevector.h"
distancevector::distancevector()
{

}
std::vector<int> distancevector::createDistanceVector(int numrouters,std::vector<init_payload_router_entry*> routers)
{
    // Initilize Distance Vector to size of numrouters + 1
    // Size of numrouters + 1 because ID's start 1
    std::vector<int> v;
    for(int i = 0; i < numrouters + 1; i++){
        v.push_back(-1);
    }

    // Build Distance Vector
    for (int i = 1; i <=numrouters; i++){
        int id = i;
        int cost = ntohs(routers[i]->cost);

        v[i] = cost;
    }
    
    return v;
}
std::vector<int> distancevector::updateDistanceVector(std::vector<table_entry> forwarding_table,std::vector<int> dv)
{
    std::vector<int> newdv = dv;
    for(int i = 1; i < forwarding_table.size(); i++){
        int newcost = forwarding_table[i].cost;
        newdv[i] = newcost;
    }
    return newdv;
}