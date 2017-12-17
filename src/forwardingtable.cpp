#include "../include/forwardingtable.h"

/*
    Initialize Forwarding Table from only known Distance Vector - Routers Own DV
*/
std::vector<table_entry> createForwardingTable(std::vector<int> dv){
        std::vector<table_entry> table;
        std::cout << "**Creating Initial Forwarding Table\n";
        // First time creating forwarding table setup based on distance vector
        for(int i = 0; i < dv.size();i++){
            if(i == 0){
                //push null table
                table_entry t;
                table.push_back(t);
            }else{
                // start at id 1
                table_entry t;
                t.routerid = i;
                t.cost = dv[i];
                
                if(t.cost >= INFINITY){
                    t.next_hop_id = INFINITY;
                }else{
                    t.next_hop_id = i;
                }
                table.push_back(t);
            }
        }
        return table;
}

/*
    Update Forwarding Table - Router recvd new DV
*/
std::vector<table_entry> updateForwardingTable(std::map<int,neighbor> neighbors,int numrouters,int srcid,std::vector<int> self_dv){
    std::cout << "**Updating Forwarding Table\n";
    std::vector<table_entry> table;
    for(int i = 0; i < numrouters + 1; i ++){
        table_entry t;
        if(i > 0){
            t.cost = self_dv[i];
            t.routerid = i;
            table.push_back(t);
        }else{
            table.push_back(t);
        }
    }

    // Run Bellman ford to find least cost path
    if(neighbors.size() != 0){
        std::map<int,neighbor>::iterator it = neighbors.begin();

        std::pair<int,int> p;
        for(int i = 0; i < numrouters; i ++){
            int y = i+1;
            int x = srcid;

            p.first = table[y].cost;
            p.second = INFINITY;

            std::map<int,neighbor>::iterator itt;

            for(itt = neighbors.begin(); itt != neighbors.end(); itt++){
                std::vector<int> neighborDV = itt->second.dv;
                if(neighborDV.size() == 0){
                    // Do Nothing - don't have this DV yet - this really shouldn't happen but just in case
                    // so the program does not crash
                    std::cout << "Error - no DV for neighbor yet\n";
                }else{
                    int neighborid = itt->first;
                    int cost = self_dv[neighborid] + neighborDV[y];
                    int next_hop = neighborid;

                    // Check if lowest found
                    if(cost <= p.first){
                        p.first = cost;
                        p.second = next_hop;
                    }
                }

            }
                // Check if lowest cost since last cost and save
                table_entry t;
                t.routerid = y;
                t.next_hop_id = p.second;
                t.cost = p.first;
                table[t.routerid] = t;
                if(table[t.routerid].cost == 0){
                    table[t.routerid].next_hop_id = srcid;

                }
        }
    }else{
        // No neighbors , do nothing. Forwarding table is current DV

    }

    return table;
}