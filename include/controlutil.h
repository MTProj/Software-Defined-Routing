#ifndef CONTROLUTIL_H_
#define CONTROLUTIL_H_


#include "../include/packetlib.h"
#include "../include/router.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <map>


class controlutil{
    private:

    public:
    controlutil();

    control_message_header* processHeader(char* buffer);
    char* createResponseHeader(int sock,uint8_t control_code, uint8_t response_code,uint16_t payload_length);

};



#endif