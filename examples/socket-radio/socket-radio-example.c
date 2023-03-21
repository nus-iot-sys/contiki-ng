#include "contiki.h"
#include "net/netstack.h"
#include "process.h"
#include "socket-radio.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PROCESS(socket_radio_example_process, "Socket Radio Example");
AUTOSTART_PROCESSES(&socket_radio_example_process);

#define BUFSZ  128u
static char buf[BUFSZ];

void input_callback(const void *data, uint16_t len)
{
    LOG_INFO("Received from socket radio: %s (len=%d)\n", (char *)data, len);
    LOG_INFO("Sending echo\n");

    NETSTACK_RADIO.transmit(len);
}


PROCESS_THREAD(socket_radio_example_process, ev, data)
{

    PROCESS_EXITHANDLER(NETSTACK_RADIO.off(););

    PROCESS_BEGIN();
    
    socket_radio_set_input_callback(input_callback);
    socket_radio_buf = (uint8_t *)buf;
    socket_radio_bufsz = BUFSZ;

    NETSTACK_RADIO.on();


    PROCESS_END();
}


