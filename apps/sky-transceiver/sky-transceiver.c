#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/serial-line.h"

#include <stdio.h>
#include <string.h>

PROCESS(sky_transceiver_process, "Sky transceiver process");
AUTOSTART_PROCESSES(&sky_transceiver_process);

static void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("%s\n", (char *)data);
}


PROCESS_THREAD(sky_transceiver_process, ev, data)
{
  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
    nullnet_len = strlen(data) + 1;
    nullnet_buf = data;
    NETSTACK_NETWORK.output(NULL);
  }

  PROCESS_END();
}
