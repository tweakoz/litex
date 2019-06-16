#ifndef __TFTP_H
#define __TFTP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* c_str_t;

int tftp_get(uint32_t ip, uint16_t server_port, c_str_t filename, uint8_t* outbuffer, uint32_t expected_len );
int tftp_put(uint32_t ip, uint16_t server_port, c_str_t filename, const uint8_t* buffer, uint32_t size );

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* __TFTP_H */
