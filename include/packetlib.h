#ifndef PACKETLIB_H_
#define PACKETLIB_H_

#include <stdint.h>
#include <string>

/* Control Message/Response Headers/Payloads */
static int CONTROL_HEADER_SIZE = 8;
static int CONTROL_HEADER_RESPONSE_SIZE = 8;
static int INIT_PAYLOAD_HEADER_SIZE = 4;
static int INIT_PAYLOAD_ROUTER_ENTRY_SIZE = 12;
static int ROUTING_UPDATE_HEADER_SIZE = 8;
static int ROUTING_UPDATE_ENTRY_SIZE = 12;
static int ROUTING_TABLE_PAYLOAD_ENTRY_SIZE = 8;
static int CONTROL_UPDATE_PAYLOAD_SIZE = 4;

struct control_message_header
{
    uint32_t dest_ip_addr;
    uint8_t control_code;
    uint8_t response_time;
    uint16_t payload_len;
};
struct control_response_message_header
{
    uint32_t controller_ip;
    uint8_t control_code;
    uint8_t response_code;
    uint16_t payload_len;
};
/* 
   Distance Vector Update Header
   8 Bytes
*/
struct routing_update_header
{
    uint16_t num_update_fields;
    uint16_t source_router_port;
    uint32_t source_router_ip;
};
/* Distance Vector Update Entries
   12 Bytes Each
*/
struct routing_update_entry{
    uint32_t router_ip;
    uint16_t router_port;
    uint16_t padding;
    uint16_t id;
    uint16_t cost;
};
struct routing_table_entry{
    uint16_t id;
    uint16_t padding;
    uint16_t next_hop_id;
    uint16_t cost;
};

/* INIT Packet Structs */
struct init_payload_header{
    /* 
    Payload Header Entry - 4 Bytes
    | Num Routers [16 Bits] | Update Interval [16 Bits] |
    */
    uint16_t num_routers;
    uint16_t update_interval;
};
struct init_payload_router_entry{
    /* 
    Payload Entry Sent By Controller - 12 Bytes

    | ID      [16 Bits] |  Port-1  [16 Bits] |
    | Port-2  [16 Bits] |  Cost    [16 Bits] |
    |       IP Address[32 Bits]              |

    */
    uint16_t id;
    uint16_t port1;
    uint16_t port2;
    uint16_t cost;
    uint32_t ip;
};
struct routing_table_payload_entry{
    uint16_t id;
    uint16_t padding;
    uint16_t next_hop_id;
    uint16_t cost;
};
struct control_update_payload{
    uint16_t id;
    uint16_t cost;
};
/*
    Controller Sends a file the payload of the packet will contain the following
    The filename is just a variable after this.
    |               Dest IP [32 bits]                       |
    | TTL [8 bits] | T_ID [8 bits] | Init Seq Num [16 bits] |
    |               FILENAME - VARIABLE - STRING?           |

*/
struct sendfile_payload_header{
    uint32_t dest_ip;
    uint8_t ttl;
    uint8_t transfer_id;
    uint16_t init_seq_num;
};
/*
    Controller Sends this to get file stats
    | Transfer ID [8 bits] |
*/
struct sendfile_stats_payload{
    uint8_t transferID;
};
struct sendfile_stats_response_payload_header{
    uint8_t transfer_id;
    uint8_t ttl;
    uint16_t padding;
};
struct sendfile_stats_response_payload_entry{
    int seqnum;
};
// Data Plane Header/Packet
struct data_plane_header
{


};

#endif