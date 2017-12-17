#include "../include/controlutil.h"
#include "../include/router.h"

#include <iostream>
#include <netinet/in.h>
#include <map>


controlutil::controlutil(){

}
control_message_header* controlutil::processHeader(char *buffer){
    control_message_header* header = reinterpret_cast<control_message_header*>(buffer);
    return header;
}

char* controlutil::createResponseHeader(int sock,uint8_t control_code, uint8_t response_code,uint16_t payload_length){

    char *buffer;
    control_response_message_header *h;
    struct sockaddr_in addr;
    socklen_t addr_size;

    // Allocated memory to size of control response header
    buffer = new char[CONTROL_HEADER_SIZE];
    h = reinterpret_cast<control_response_message_header*>(buffer);

    // Get address information for controller
    addr_size = sizeof(struct control_response_message_header);
    getpeername(sock,(struct sockaddr *)&addr,&addr_size);

    // Build Header
    memcpy(&(h->controller_ip),&(addr.sin_addr),sizeof(struct in_addr));
    h->control_code = control_code;
    h->response_code = response_code;
    h->payload_len = htons(payload_length);

    return buffer;
}
