#include "bsp_uart.h"
#include "usart.h"   // 如果你的 CubeMX 生成了这个头文件
#include "common_def.h"

int bsp_uart_send(uint8_t *buf, uint16_t len)
{
    if (buf == 0U || len == 0U)
    {
        return RET_INVALID_PARAM;
    }

    if (HAL_UART_Transmit(&huart3, buf, len, 1000) != HAL_OK)
    {
        return RET_ERR;
    }

    return RET_OK;
}
