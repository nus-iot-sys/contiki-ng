#include "cc.h"
#include "clock.h"
#include "contiki.h"
#include "etimer.h"
#include "msp430-def.h"
#include "net/netstack.h"
#include "packetbuf.h"
#include "process.h"
#include "radio.h"
#include "sys/node-id.h"
#include "dev/radio/cc2420/cc2420.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

PROCESS(collision_recovery_process, "Collision Recovery Example");
PROCESS(sender_process, "Sender");
PROCESS(receiver_process, "Receiver");
AUTOSTART_PROCESSES(&collision_recovery_process);

static void input_callback(const void *data, uint16_t len)
{
  if (len == 0) {
    printf("data_len=0\n");
    return;
  }
  int str_len = strlen((char *)data);
  if (len != str_len + 1) {
    printf("data_len=%d, str_len=%d\n", len, str_len);
  } else {
    printf("Received: %s\n", (char *)data);
  }
}

PROCESS_THREAD(collision_recovery_process, ev, data)
{
  radio_value_t radio_tx_mode;

  PROCESS_BEGIN();

  cc2420_set_input_callback(input_callback);
  NETSTACK_RADIO.on();
  NETSTACK_RADIO.get_value(RADIO_PARAM_TX_MODE, &radio_tx_mode);
  NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE,  radio_tx_mode | (~RADIO_TX_MODE_SEND_ON_CCA));

  if (node_id > 1) { /* Sender */
    process_start(&sender_process, NULL);
  } else {
    process_start(&receiver_process, NULL);
  }

  PROCESS_END();
}

PROCESS_THREAD(sender_process, ev, data)
{
  static struct etimer timer;
  static unsigned count;
  static char buf[128];
  
  PROCESS_BEGIN();
  
  count = 0;

  etimer_set(&timer, (node_id - 1) * CLOCK_SECOND / 20);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    sprintf(buf, " longlonglonglonglonglonglonglonglongMessage %d from %d", count, node_id);
    NETSTACK_RADIO.send(buf, strlen(buf) + 1);
    count++;
    etimer_reset(&timer);
  }

  PROCESS_END();
}

PROCESS_THREAD(receiver_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    PROCESS_YIELD();
  }
  PROCESS_END();
}