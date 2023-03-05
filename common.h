#ifndef COMMON_H
#define COMMON_H

#define SIZE_OF_TXBUFF  1426        // Header Size 54 Bytes, Paket Lang - 1480
#define SIZE_OF_RXBUFF  32

typedef struct {
  int       socket;
  struct    sockaddr_in addres;

  //message_t sending_buffer;
  char      tx_buff[SIZE_OF_TXBUFF];
  size_t    to_send;

  /* The same for the receiving message. */
  char      rx_buff[SIZE_OF_RXBUFF];
} peer_t;

#endif /* COMMON_H */
