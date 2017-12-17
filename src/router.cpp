#include "../include/router.h"
#include "../include/packetlib.h"
#include "../include/controlutil.h"
#include "../include/network.h"
#include "../include/distancevector.h"
#include "../include/forwardingtable.h"

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <time.h>

router::router()
{
    // Initialize router and data socket. These will be changed later.
    router_sock = -1;
    data_listener = -1;
    control_sock = -1;
    sockaddr_in control_addr;
    CONTROLLER_ACTIVE = false;
    TIMEOUT.tv_sec = 20;

}
/* Main Loop */
void router::startRouter(int CONTROL_PORT)
{
    // Setup TCP Control Listener
    createControllerSocketListener(CONTROL_PORT);
    fd_set socklist;
    int maxfd;
    
    while(true){
        // Reset FD List
        FD_ZERO(&socklist);
        FD_SET(control_listener,&socklist);
        FD_SET(router_sock,&socklist);
        FD_SET(data_listener,&socklist);
        FD_SET(control_sock,&socklist);
        for(int i = 0; i < data_plane_routers.size(); i++){
            int s = data_plane_routers[i];
            FD_SET(s,&socklist);
        }

        // Find max fd
        maxfd = control_listener;
        if(router_sock > maxfd){ maxfd = router_sock; }
        if(data_listener > maxfd){ maxfd = data_listener; }
        if(control_sock > maxfd){ maxfd = control_sock; }
        for(int i = 0; i < data_plane_routers.size(); i++){
            int s = data_plane_routers[i];
            if(s > maxfd){
                maxfd = s;
            }
        }


        // Wait for Active Socket
        int ready = select(maxfd + 1,&socklist,NULL,NULL,&TIMEOUT);
        if(ready == -1){
            //std::cout << "Error with select\n\n\n";
        }else if(ready == 0){
            timerInterrupt();
        }else{
            if(FD_ISSET(control_listener,&socklist)){
                // Control is trying to make a connection with this router - accept
                size_t control_addr_len = sizeof(control_addr);
                control_sock = accept(control_listener,(struct sockaddr*)&control_addr,(socklen_t*)&control_addr_len);
                CONTROLLER_ACTIVE = true;

            }else if(FD_ISSET(data_listener,&socklist)){
                // Router Trying to connect to transfer data via TCP
                size_t router_addr_len = sizeof(router_addr_len);
                sockaddr_in router_addr;
                int sock = accept(data_listener,(struct sockaddr*)&router_addr,(socklen_t*)&router_addr_len);
                addDataPlaneRouterSocket(sock);

            }else if (FD_ISSET(router_sock,&socklist)){
                /*
                    This will be a routing update when receiving an update, depending on router 
                */
                struct sockaddr_in addr;
                socklen_t addrlen = sizeof(addr);
                char *buffer = new char[ROUTING_UPDATE_HEADER_SIZE + (NUMBER_OF_ROUTERS * ROUTING_UPDATE_ENTRY_SIZE)];
                int ret = recvfrom(router_sock,buffer,1024,0,(struct sockaddr *)&addr,&addrlen);
                processDistanceVectorUpdate(buffer);
                


            }else if(FD_ISSET(control_sock,&socklist)){
                controlutil control;

                // Allocate new buffer for header
                char* control_header = new char[CONTROL_HEADER_SIZE];
                if(recvAll(control_sock,control_header,CONTROL_HEADER_SIZE) <= 0){
                    closeControllerSocket();
                }

                if(CONTROLLER_ACTIVE){

                    // Build Header From Buffer
                    control_message_header* header = reinterpret_cast<control_message_header*>(control_header);
                    uint8_t control_code = header->control_code;
                    uint16_t payload_len = ntohs(header->payload_len);
                    uint8_t response_time = header->response_time;

                    char* control_payload = new char[header->payload_len];

                    // Free Memory
                    delete [] control_header;

                    // Get Payload if there is one
                    if(payload_len != 0){

                        if(recvAll(control_sock,control_payload,payload_len) <= 0){
                            closeControllerSocket();
                        }
                    }

                    // Switch on Control code
                    switch(control_code){
                        case 0:
                        {
                            // Create Payload for Response to Controller
                            std::string AUTHOR = "Mitchelt";
                            sendControlResponseMessage(control_sock,control_code,0x00,AUTHOR);
                        };
                        break;
                    
                        case 1:
                        {
                            // INIT 0x01
                            //std::cout << "  Control Code Received: INIT \n";

                            // Process Init Message
                            processInitPayload(control_payload);
                            sendControlResponseMessage(control_sock,control_code,0x00,"");
                        };
                        break;

                        case 2:
                        {
                            // ROUTING-TABLE 0x02
                            std::cout << "Control Code Received: ROUTING-TABLE \n";
                            sendRoutingTableResponse(control_sock,control_code,0x00,forwarding_table);
                            // Create Header
                        };
                        
                        break;

                        case 3:
                        // UPDATE 0x03
                            processUpdatePayload(control_payload);
                            sendControlResponseMessage(control_sock,control_code,0x00,"");
                        break;

                        case 4:
                        // CRASH 0x04
                            sendControlResponseMessage(control_sock,control_code,0x00,"");
                            exit(0);

                        break;

                        case 5:
                        // SENDFILE 0x05
                        // TODO:

                        break;

                        case 6:
                        // SENDFILE-STATS 0x06
                        // TODO:

                        break;

                        case 7:
                        // LAST-DATA-PACKET 0x07
                        // TODO:

                        break;

                        case 8:
                        // PENULTIMATE-DATA-PACKET 0x08
                        // TODO:
                        break;
                    }

                }
            }else{
                // Router Data Plane socket is active - find which is active
                int active_socket;
                for(int i = 0; i < data_plane_routers[i]; i++){
                    int s = data_plane_routers[i];
                    if(FD_ISSET(s,&socklist)){
                        active_socket = s;
                    }
                }
                // TODO:

            }
        }

    }
}
void router::closeControllerSocket(){
    control_sock = -1;
    CONTROLLER_ACTIVE = false;
}
void router::closeDataPlaneRouterSocket(int sock){
    for(int i = 0; i < data_plane_routers.size(); i++){
        if(sock == data_plane_routers[i]){
            data_plane_routers[i] = -1;
            break;
        }
    }
}
void router::addDataPlaneRouterSocket(int sock){
    for(int i = 0; i < data_plane_routers.size(); i++){
        if(data_plane_routers[i] == -1){
            data_plane_routers[i] = sock;
            break;
        }
    }
}
/* UDP PORT for Router to Router communication */
void router::createRouterSocket(int portnum){
    router_sock = socket(AF_INET,SOCK_DGRAM,0);
    if(router_sock < 0 ){
        std::cout << "Unable to create UDP socket\n";
    }else{

        ROUTER_UDP_ADDR.sin_family = AF_INET;
        ROUTER_UDP_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);
        ROUTER_UDP_ADDR.sin_port = htons(portnum);

        if(bind(router_sock,(struct sockaddr*) &ROUTER_UDP_ADDR,sizeof(ROUTER_UDP_ADDR)) < 0){
            std::cout << "Unable to bind UDP socket to port " << portnum << "\n";
        }else{
            std::cout << "**UDP Socket open on " << portnum << "\n";
        }

    }
}
/* TCP Socket Listener for Data plane */
void router::createDataSocketListener(int portnum){

    data_listener = socket(AF_INET,SOCK_STREAM,0);
    if(data_listener < 0 ){
        std::cout << "Unable to create TCP Data socket Listener\n";
    }else{



        DATA_TCP_LISTEN_ADDR.sin_family = AF_INET;
        DATA_TCP_LISTEN_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);
        DATA_TCP_LISTEN_ADDR.sin_port = htons(portnum);

        if(bind(data_listener,(struct sockaddr*) &DATA_TCP_LISTEN_ADDR,sizeof(DATA_TCP_LISTEN_ADDR)) < 0){
            std::cout << "Unable to bind TCP Data Plane socket to port " << portnum << "\n";
        }

        int ret = listen(data_listener,0);
        if(ret < 0){
            std::cout << "Unable to List to on Data Port\n";
        }else{
            std::cout << "**Data Socket Listening on " << portnum << "\n";
        }
    }
}

/* TCP Controller Listener */
void router::createControllerSocketListener(int portnum){
    control_listener = socket(AF_INET,SOCK_STREAM,0);
    if(control_listener < 0 ){
        std::cout << "Unable to create TCP Controller Listener\n";
    }else{
        CONTROL_TCP_LISTEN_ADDR.sin_family = AF_INET;
        CONTROL_TCP_LISTEN_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);
        CONTROL_TCP_LISTEN_ADDR.sin_port = htons(portnum);

        if(bind(control_listener,(struct sockaddr*) &CONTROL_TCP_LISTEN_ADDR,sizeof(CONTROL_TCP_LISTEN_ADDR)) < 0){
            std::cout << "Unable to bind TCP  Control Listen socket to port " << portnum << "\n";
        }

        int ret = listen(control_listener,0);
        if(ret < 0){
            std::cout << "Eror unable to listen with control listener\n";
            exit(EXIT_FAILURE);
        }else{
            std::cout << "**Controller Socket Listening on " << portnum << "\n";
        }
    }
}
init_payload_router_entry* router::getRouterInfo(int id){
    return routers[id];
}
void router::setRouterInfo(int id,init_payload_router_entry *r){
    routers[id] = r;

}
void router::processUpdatePayload(char* control_payload){
    char *buffer = new char[CONTROL_UPDATE_PAYLOAD_SIZE];
    memcpy(buffer,control_payload,CONTROL_UPDATE_PAYLOAD_SIZE);
    control_update_payload* p = reinterpret_cast<control_update_payload*>(buffer);

    // Set New Value
    int id = ntohs(p->id);
    int cost = ntohs(p->cost);

    //Update in forwarding table
    forwarding_table[id].cost = cost;
    routers[id]->cost = cost;
    distance_vector[id] = cost;
}
/* 
    INIT Payload from controller contains all necessary information to get the router up and running 
    Graph of Network
    Number of Nodes
    Update Interval
*/
void router::processInitPayload(char* control_payload){

    /*
        The Payload from the init message contains Preliminary information before 
        the router entries - need to pull this info before getting all router entries.

        4 Bytes of data - Number of routers and Update interval

        Set NUMBER_OF_ROUTERS,UPDATE_INTERVAL, and TIMEOUT.tv_sec
    */
    char* buffer = new char [INIT_PAYLOAD_HEADER_SIZE];
    memcpy(buffer,control_payload,INIT_PAYLOAD_HEADER_SIZE);
    init_payload_header* p = reinterpret_cast<init_payload_header*>(buffer);

    NUMBER_OF_ROUTERS = ntohs(p->num_routers);
    UPDATE_INTERVAL = ntohs(p->update_interval);
    TIMEOUT.tv_sec = UPDATE_INTERVAL;

    // Initialize routers and data plane routers arrays
    for(int i = 0; i < NUMBER_OF_ROUTERS + 1; i++){
        init_payload_router_entry* r;
        routers.push_back(r);
    }
    for(int i = 0; i < NUMBER_OF_ROUTERS; i++){
        data_plane_routers.push_back(-1);
    }

    delete [] buffer;

    // Get the router entries
    // Offset is set to 4 because of previous operation.
    int offset = 4;
    for(int i = 0; i < NUMBER_OF_ROUTERS; i++){

        init_payload_router_entry *r;
        char *buffer = new char[INIT_PAYLOAD_ROUTER_ENTRY_SIZE];
        memcpy(buffer,control_payload+offset,INIT_PAYLOAD_ROUTER_ENTRY_SIZE);
        r = reinterpret_cast<init_payload_router_entry*>(buffer);

        struct in_addr ip_addr;
        ip_addr.s_addr = r->ip;

        int COST = ntohs(r->cost);

        // Set ROUTER_ID - This is to recognize self in router vector
        if(COST == 0){

            // This Router Entry is THIS router - set ID and create proper sockets
            ROUTER_ID = ntohs(r->id);
            int port1 = ntohs(r->port1);
            int port2 = ntohs(r->port2);
            createRouterSocket(port1);
            createDataSocketListener(port2);

            neighbor n;
            n.id = ROUTER_ID;
            std::pair<int,neighbor> p;
            p.first = n.id;
            p.second = n;

            // Add to deque
            routing_updates.push_back(p);



        
        }else{
            // Add router and info to router_sendto map - this contains non uint data types.
            // Easier to work with

            router_entry rout;
            rout.id = ntohs(r->id);
            rout.router_port = ntohs(r->port1);
            rout.data_port = ntohs(r->port2);

            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = r->ip;

            rout.ip = inet_ntoa(addr.sin_addr);

            rout.addr = addr;
            
            std::pair<int,router_entry> p;
            p.first = rout.id;
            p.second = rout;
            routers_sendto.insert(p);

            // If router cost is not infinity add to neighbors map
            if(COST != INFINITY){
                neighbor n;
                n.id = rout.id;
                n.online = true;
                n.missedupd = 0;

                std::pair<int,neighbor> p;
                p.first = n.id;
                p.second = n;
                neighbors.insert(p);
            }
        }
        //Add router to routers data structure
        routers[i+1] = r;
        offset = offset + 12;
    }
    // Create Initial Distance Vector and update forwarding table
    distancevector d;
    distance_vector = d.createDistanceVector(NUMBER_OF_ROUTERS,routers);
    forwarding_table = createForwardingTable(distance_vector);
    //printForwardingTable();
}
void router::sendControlResponseMessage(int control_sock, uint8_t control_code,uint8_t response_code,std::string payload)
{
    controlutil control;

    // Send Response Packet
    uint16_t packet_len;
    uint16_t payload_len;

    char *header;
    char *packet;
    
    payload_len = payload.size();
    packet_len = CONTROL_HEADER_SIZE + payload_len;

    // Create Header 
    header = control.createResponseHeader(control_sock,control_code,response_code,payload_len);

    // Allocate Memory for Packet
    packet = new char[packet_len];

    // Copy header to packet
    memcpy(packet,header,8);
    delete [] header;

    if(payload_len != 0){
        // Copy Payload to packet
        char *buffer = new char[payload_len];
        memcpy(buffer,payload.c_str(),payload_len);
        memcpy(packet + CONTROL_HEADER_SIZE,buffer,payload_len);
        delete [] buffer;
    }
    sendAll(control_sock,packet,packet_len);

}
void router::sendRoutingTableResponse(int control_sock,uint8_t control_code,uint8_t response_code,std::vector<table_entry> ft){
    // Create Header
    controlutil c;
    uint16_t payload_len = ROUTING_TABLE_PAYLOAD_ENTRY_SIZE * NUMBER_OF_ROUTERS;
    char *header = c.createResponseHeader(control_sock,control_code,0x00,payload_len);

    // Create Payload
    char *payload = new char[ROUTING_TABLE_PAYLOAD_ENTRY_SIZE * NUMBER_OF_ROUTERS];
    int offset = 0;

    for(int i = 1; i < ft.size(); i++){
        char *entry = new char[ROUTING_TABLE_PAYLOAD_ENTRY_SIZE];
        routing_table_payload_entry* p;
        p = reinterpret_cast<routing_table_payload_entry*>(entry);
        p->id = htons(ft[i].routerid);
        p->padding = 0x00;
        p->next_hop_id = htons(ft[i].next_hop_id);
        p->cost = htons(ft[i].cost);

        memcpy(payload+offset,entry,ROUTING_TABLE_PAYLOAD_ENTRY_SIZE);
        offset = offset + ROUTING_TABLE_PAYLOAD_ENTRY_SIZE;
        delete [] entry;
    }

    // Create Packet Buffer
    char *packet = new char[payload_len + CONTROL_HEADER_RESPONSE_SIZE];

    //Copy data to packet and send
    memcpy(packet,header,CONTROL_HEADER_RESPONSE_SIZE);
    delete [] header;
    memcpy(packet+CONTROL_HEADER_RESPONSE_SIZE,payload,payload_len);
    delete [] payload;
    
    int packet_len = payload_len + CONTROL_HEADER_RESPONSE_SIZE;
    //Send data out
    int ret = sendAll(control_sock,packet,packet_len);
}

void router::broadcastDistanceVector(){
    // Allocate memory for packet buffer
    int packet_size = (ROUTING_UPDATE_ENTRY_SIZE * NUMBER_OF_ROUTERS) + ROUTING_UPDATE_HEADER_SIZE;
    char *packet = new char[packet_size];

    // Build Header for Packet
    char *header = new char[ROUTING_UPDATE_HEADER_SIZE];
    routing_update_header *h;
    h = reinterpret_cast<routing_update_header*>(header);
    
    h->num_update_fields = htons(distance_vector.size() - 1);
    h->source_router_port = htons(routers[ROUTER_ID]->port1);
    h->source_router_ip = htonl(routers[ROUTER_ID]->ip);

    // Copy header to packet
    memcpy(packet,header,ROUTING_UPDATE_HEADER_SIZE);
    delete [] header;

    // Build Router Entries
    int offset = ROUTING_UPDATE_HEADER_SIZE;
    for(int i = 1; i < distance_vector.size(); i++){     
        char *entry = new char[ROUTING_UPDATE_ENTRY_SIZE];
        routing_update_entry* r;
        r = reinterpret_cast<routing_update_entry*>(entry);
        r->id = htons(i);
        r->cost = htons(distance_vector[i]);
        r->router_ip = htonl(routers[i]->ip);
        r->router_port = htons(routers[i]->port1);
        r->padding = htons(0x00);

        // Copy entry to packet
        memcpy(packet + offset,entry,ROUTING_UPDATE_ENTRY_SIZE);
        delete [] entry;

        // Increment Offset
        offset = offset + ROUTING_UPDATE_ENTRY_SIZE;
    }

    // Send packet to all neighbors
    std::map<int,neighbor>::iterator it;
    for(it = neighbors.begin(); it != neighbors.end(); it ++){
        if(neighbors[it->second.id].online == false){
            // Do Not Send Routing Update
        }else{
            // Neighbor is online send vector update
            int sock = socket(AF_INET,SOCK_DGRAM,0);
            int id = it->second.id;
            struct sockaddr_in dest = routers_sendto[id].addr;
            dest.sin_port = htons(routers_sendto[id].router_port);
            int ret = sendto(sock,packet,packet_size,0,(struct sockaddr*)&dest,sizeof(dest));

        }

    }
    delete [] packet;
}
void router::processDistanceVectorUpdate(char* buffer){
    
    // Get Header
    char *header = new char [ROUTING_UPDATE_HEADER_SIZE];
    memcpy(header,buffer,ROUTING_UPDATE_HEADER_SIZE);
    routing_update_header *h = reinterpret_cast<routing_update_header*>(header);
    int num_entries = ntohs(h->num_update_fields);
    int source_port = ntohs(h->source_router_port);

    struct in_addr ip_addr;
    ip_addr.s_addr = h->source_router_ip;

    delete [] header;

    // Initialize Distance Vector 
    std::vector<int> newdv;
    for(int i = 0; i < num_entries + 1; i++){
        newdv.push_back(INFINITY);
    }

    // Build New DV
    int sourceid;
    int offset = ROUTING_UPDATE_HEADER_SIZE;
    for(int i = 0; i < num_entries; i++){
        char *entry = new char[ROUTING_UPDATE_ENTRY_SIZE];
        memcpy(entry,buffer+offset,ROUTING_UPDATE_ENTRY_SIZE);
        routing_update_entry *r = reinterpret_cast<routing_update_entry*>(entry);

        int routerid = ntohs(r->id);
        int cost = ntohs(r->cost);
        newdv[routerid] = cost;

        offset = offset + ROUTING_UPDATE_ENTRY_SIZE;
        if(cost == 0){
            sourceid = i+1;
        }
    }

    // Check if this DV is different from current known DV
    std::vector<int> currentdv = neighbors[sourceid].dv;
    if(currentdv.size() == 0){
        // First time receiving DV from router 
        // Update dv in neighbor
        // Add to routing_updates queue - set time to now.
        neighbors[sourceid].dv = newdv;
        neighbors[sourceid].id = sourceid;
        neighbors[sourceid].online = true;
        neighbors[sourceid].missedupd = 0;
        time_t t;
        neighbors[sourceid].last_time = time(&t);

        std::pair<int,neighbor> p;
        p.first = neighbors[sourceid].id;
        p.second = neighbors[sourceid];
        routing_updates.push_back(p);

    }else{
        // Check for changes between current and next dv
        for(int i = 1; i < currentdv.size(); i ++){
            if(currentdv[i] != newdv[i]){
                // there is a difference , update dv and exit loop
                neighbors[sourceid].dv = newdv;
                break;
            }
        }
        
        // Remove from dequeue and add to the end with updated time
        // Reset missedupd counter
        neighbors[sourceid].missedupd = 0;
        time_t t;
        neighbors[sourceid].last_time = time(&t);

        // Iterate through dequeue to find source router in routing updates
        std::deque<std::pair<int,neighbor> >::iterator it;
        for(it = routing_updates.begin(); it != routing_updates.end(); it++){
            if(it->first == sourceid){

                // Erase from routing updates and re-add to end of deque with updated time.
                routing_updates.erase(it);
                std::pair<int,neighbor> p;
                p.first = sourceid;
                p.second = neighbors[sourceid];
                routing_updates.push_back(p);
                break;
            }   
        }
    }
    // Create Initial Forwarding
    distancevector d;
    forwarding_table = updateForwardingTable(neighbors,NUMBER_OF_ROUTERS,ROUTER_ID,distance_vector);
    distance_vector = d.updateDistanceVector(forwarding_table,distance_vector);
}

/* Timer Interrupt */
void router::timerInterrupt(){
    // Timer Interrupt
    time_t interrupt_time;
    time(&interrupt_time);

    if(routing_updates.size() == 0){
        // This should actually never happen but just in case. The SELF router should always be in the queu
        // to send the routing updates out.
        std::cout << "Routing Update Queue is empty for some reason..\n";
    }else{
        if(routing_updates.front().first == ROUTER_ID){
            // Broadcast distance vector to all neighbors
            broadcastDistanceVector();

            neighbor n = routing_updates.front().second;

            //Update time in struct
            n.last_time = interrupt_time;

            //Pop from front of queue
            routing_updates.pop_front();

            //Add to end of queue
            std::pair<int,neighbor> p;
            p.first = n.id;
            p.second = n;
            routing_updates.push_back(p);
        }else{
            neighbor n = routing_updates.front().second;
            
            // Pop neighbor from top of queue
            routing_updates.pop_front();

            // Increase missedupd by 1
            n.missedupd++;
            if(n.missedupd >= 3){
                // Router failed to send DV for 3 consecutive update intervals - mark offline and cost infinity
                n.online = false;
                neighbors[n.id].online = false;
                distance_vector[n.id] = INFINITY; 
                                
            }else{
                // Router failed to send DV but it has not yet been 3 consecutive. Update time with interrupt time
                n.last_time = interrupt_time;

                // Add to end of queue
                std::pair<int,neighbor> p;
                p.first = n.id;
                p.second = n;
                routing_updates.push_back(p);
            }
        }
        // Update Select TIMEOUT. Based off next neighbor entry in queue
        int DIFF = interrupt_time - routing_updates.front().second.last_time;
        TIMEOUT.tv_sec = UPDATE_INTERVAL - DIFF;
    }
}
/*
    Print Functions
*/
void router::printNeighborMap(){
    std::cout << "      Neighbor Map\n";
    std::map<int,neighbor>::iterator it;

    for(it=neighbors.begin(); it!= neighbors.end(); it++){
        std::cout << "      ID: " << it->first << "\n";
        std::cout << "      Online: " << it->second.online << "\n";
        std::cout << "      Missed Updates: " << it->second.missedupd << "\n";
    }
}
void router::printRouterSendTo(){
    std::cout << "      Routers Sendto Map\n";
    std::map<int,router_entry>::iterator it;

    for(it=routers_sendto.begin(); it!= routers_sendto.end(); it++){
        std::cout << "      ID: " << it->first << "\n";
        std::cout << "      Router Port: " << it->second.router_port << "\n";
        std::cout << "      Data Port: " << it->second.data_port << "\n";
        std::cout << "      IP: " << it->second.ip << "\n\n";
    }


}
void router::printBytes(char* buffer, int numbytes){
    std::cout << "Buffer in router - ";
    for (int i = 0; i < numbytes; i++)
    {
        printf("%02x ", (unsigned int) buffer[i]);    
    }
    printf("\n");  // flush stdout

}
void router::printForwardingTable(){
    std::cout << "Current Forwarding Table\n";
    std::cout << "ID |Hop |Cost\n";
    for(int i = 1; i < forwarding_table.size(); i++){
        std::cout << i << " " << forwarding_table[i].next_hop_id << " " << forwarding_table[i].cost << "\n";
    }
}
