#ifndef UART_SERVICE_H
#define UART_SERVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int uart_service_init(void);
int uart_service_send_bytes(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* UART_SERVICE_H */
