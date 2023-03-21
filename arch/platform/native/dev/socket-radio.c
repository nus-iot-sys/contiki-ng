#include "contiki.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net/packetbuf.h"
#include "net/netstack.h"

#include "dev/radio.h"
#include "dev/socket-radio.h"

#include "sys/log.h"
#define LOG_MODULE "Radio"
#define LOG_LEVEL LOG_LEVEL_INFO

uint8_t *socket_radio_buf;
uint16_t socket_radio_bufsz;
static socket_radio_input_callback current_callback = NULL;

static int sockfd = -1;
static uint16_t buf_payload_len;

static int
init(void)
{
  socket_radio_buf = NULL;
  socket_radio_bufsz = 0;
  current_callback = NULL;
  return 0;
}

static int
prepare(const void *payload, unsigned short payload_len)
{
  if(payload_len > socket_radio_bufsz) {
    return RADIO_TX_ERR;
  }
  memcpy(socket_radio_buf, payload, payload_len);
  buf_payload_len = payload_len;

  return 0;
}

static int
transmit(unsigned short transmit_len)
{
  int sent = 0;
  sent = send(sockfd, socket_radio_buf, buf_payload_len, 0);
  if(sent < 0) {
    LOG_ERR("send()");
    return RADIO_TX_ERR;
  }
  buf_payload_len = 0;
  return RADIO_TX_OK;
}

static int
my_send(const void *payload, unsigned short payload_len)
{
  int ret = -1;

  if(prepare(payload, payload_len)) {
    return ret;
  }

  ret = transmit(payload_len);

  return ret;
}

static int
my_read(void *buf, unsigned short buf_len)
{
  return 0;
}

static int
channel_clear(void)
{
  return 1;
}

static int
receiving_packet(void)
{
  return 0;
}

static int
pending_packet(void)
{
  return 0;
}

static int
set_fd(fd_set *rset, fd_set *wset)
{
  FD_SET(sockfd, rset);
  return 1;
}

static void
handle_fd(fd_set *rset, fd_set *wset)
{
  if(FD_ISSET(sockfd, rset)) {
    int bytes = read(sockfd, socket_radio_buf, socket_radio_bufsz-1);
    if (bytes > 0) {
      LOG_DBG("Received %d bytes from socket radio\n", bytes);
      socket_radio_buf[bytes] = 0;
      buf_payload_len = bytes;
      if (current_callback != NULL) {
        current_callback(socket_radio_buf, buf_payload_len);
      } else {
        packetbuf_clear();
        memcpy(packetbuf_dataptr(), socket_radio_buf, bytes);
        NETSTACK_MAC.input();
      }
    }
  }
}

static const struct select_callback socket_radio_sock_callback = { set_fd, handle_fd };

static int
on(void)
{
    struct sockaddr_in radio_addr;
    int r;

    char *radio_port_str = getenv("CONTIKING_SOCKET_RADIO_PORT");
    short radio_port;

    if (radio_port_str == NULL || 
        (radio_port = atoi(radio_port_str)) <= 0) {
        LOG_ERR("CONTIKING_SOCKET_RADIO_PORT not set correclty\n");
        return 0;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG_ERR("socket()\n");
        return 0;
    }

    // memset(&radio_addr, 0, sizeof(radio_addr));
    radio_addr.sin_family = AF_INET;
    radio_addr.sin_port = htons(radio_port);
    inet_pton(AF_INET, "127.0.0.1", &radio_addr.sin_addr);

    if ((r = connect(sockfd, (struct sockaddr *)&radio_addr, sizeof radio_addr)) < 0) {
        LOG_ERR("connect()\n");
        return 0;
    }

    send(sockfd, "test", 1, 0);

    LOG_INFO("Using port %d as socket radio source\n", radio_port);
    
    select_set_callback(sockfd, &socket_radio_sock_callback);
    return 1;
}

static int
off(void)
{
  close(sockfd);
  sockfd = -1;
  return 1;
}
const struct radio_driver socket_radio_driver =
{
  init,
  prepare,
  transmit,
  my_send,
  my_read,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
};

void 
socket_radio_set_input_callback(socket_radio_input_callback callback)
{
  current_callback = callback;
}

