#ifndef ROUTER_H_
#define ROUTER_H_

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


#include "../include/packetlib.h"


static int INFINITY = 65535;

struct table_entry{
    int routerid;
    int next_hop_id;
    int cost;
};
struct router_entry{
    int id;
    sockaddr_in addr;
    int router_port;
    int data_port;
    std::string ip;
};
struct neighbor{
    int id;                             // Router ID
    std::vector<int> dv;                // Distance Vector
    bool online;                        // Set to True if online - False if crashed
    int missedupd;                      // Reset to zero after an update comes in - if ever 3 router has crashed.
    time_t last_time;                   // Time last recvd or interrupt
        
};


class router{

private:

    /* Local Variables */
    int router_sock;                    // UDP Socket Listener
    int control_listener;               // TCP Socket Listener
    int control_sock;                   // TCP Socket to communicate with controller
    int data_listener;                  // TCP Socket Listener
    bool CONTROLLER_ACTIVE;             // Used to keep track of connected controller
    char *LAST_PACKET_SENT;             // Last Packet Sent 
    char *PENULTIMATE_PACKET;           // Last - Last packet sent
    std::vector<int> distance_vector;   // THIS routers distance vector
    timeval TIMEOUT;                    // select() timeout value
    sockaddr_in control_addr;               // Controller Address
    sockaddr_in ROUTER_UDP_ADDR;            // UDP addr
    sockaddr_in DATA_TCP_LISTEN_ADDR;       // TCP Listener addr
    sockaddr_in CONTROL_TCP_LISTEN_ADDR;    // TCP Listener addr for controller    
    std::string AUTHOR;                     
    int UPDATE_INTERVAL;                    // Update interval for timeout - sent by init message
    int NUMBER_OF_ROUTERS;                  // Number of routers in network - sent by init message
    int ROUTER_ID;                          // THIS routerID - known by 0 cost in own dv

    /* Data Structures */
    std::vector<int> data_plane_routers;                        // Holds TCP Connected routers - for file transfer

    std::vector<init_payload_router_entry*> routers;            // Hold initial payload router entries - uint data
    std::map<int,router_entry> routers_sendto;                  // Holds int / struct value to reference for sending
    std::map<int,neighbor> neighbors;                           // Holds routers directly connected to this router
    std::deque<std::pair<int,neighbor> > routing_updates;       // Queue to keep track of next router that will interrupt (self or neighbor)       
    std::vector<table_entry> forwarding_table;                  // Routing forwarding build from Distance Vectors received

    /* Functions */
    void closeControllerSocket();
    void closeDataPlaneRouterSocket(int sock);
    void addDataPlaneRouterSocket(int sock);
    void createRouterSocket(int portnum);
    void createDataSocketListener(int portnum);
    void createControllerSocketListener(int portnum);
    void initRoutersStructure(int numrouters);
    init_payload_router_entry* getRouterInfo(int id);
    void setRouterInfo(int id,init_payload_router_entry *r);
    void getPayloadInitInfo(char *buffer);
    void processInitPayload(char* control_payload);
    void processUpdatePayload(char *control_payload);
    void sendControlResponseMessage(int control_sock, uint8_t control_code,uint8_t response_code,std::string payload);
    void sendRoutingTableResponse(int control_sock,uint8_t control_code,uint8_t response_code,std::vector<table_entry> ft);
    void broadcastDistanceVector();
    void processDistanceVectorUpdate(char *buffer);
    void printRouterSendTo();
    void printBytes(char* buffer, int numbytes);
    void printNeighborMap();
    void printForwardingTable();
    void timerInterrupt();




public:
    router();
    void startRouter(int control_port);
};


#endif