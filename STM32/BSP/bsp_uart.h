#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int bsp_uart_send(uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
