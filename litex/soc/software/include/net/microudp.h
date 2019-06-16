#ifndef __MICROUDP_H
#define __MICROUDP_H

#include <stdint.h>

#define IPTOINT(a, b, c, d) ((a << 24)|(b << 16)|(c << 8)|d)

#define MICROUDP_BUFSIZE (5*1532)

typedef void (*udp_callback)(uint32_t src_ip, uint16_t src_port, uint16_t dst_port, void *data, uint32_t length);

#ifdef __cplusplus
extern "C" {
#endif

void microudp_start(const uint8_t *macaddr, uint32_t ip);
int microudp_arp_resolve(uint32_t ip);
uint8_t* microudp_get_tx_buffer(void);
int microudp_send(uint16_t src_port, uint16_t dst_port, uint32_t length);
void microudp_set_callback(udp_callback callback);
void microudp_service(void);

void eth_init(void);
void eth_mode(void);

#ifdef __cplusplus
} //extern "C" {
#endif

#endif /* __MICROUDP_H */
