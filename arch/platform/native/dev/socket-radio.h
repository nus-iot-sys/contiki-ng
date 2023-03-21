#ifndef SOCKET_RADIO_H_
#define SOCKET_RADIO_H_

#include "contiki.h"
#include "dev/radio.h"

#include <stdint.h>

extern const struct radio_driver socket_radio_driver;

extern uint8_t *socket_radio_buf;
extern uint16_t socket_radio_bufsz;

typedef void (* socket_radio_input_callback)(const void *data, uint16_t len);

void socket_radio_set_input_callback(socket_radio_input_callback callback);

#endif /* SOCKET_RADIO_H_ */
