#ifndef PROTO_RX_H
#define PROTO_RX_H

#include <stdint.h>
#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

void proto_rx_init(void);
void proto_rx_start(void);

/*
 * @brief  在串口接收完成回调中调用
 * @param  huart: 串口句柄
 */
void proto_rx_uart_cplt_callback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* PROTO_RX_H */
