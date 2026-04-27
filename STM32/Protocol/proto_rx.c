#include "proto_rx.h"
#include "proto_cmd.h"
#include "proto_frame.h"
#include "proto_dispatch.h"
#include "common_def.h"
#include <string.h>

/*
 * 第一版接收缓冲区
 * 长度足够覆盖当前协议的最大帧
 */
static uint8_t  s_rx_buf[PROTO_MAX_FRAME_LEN];
static uint16_t s_rx_len = 0U;

/*
 * 用于保存当前收到的一个字节
 * 一般会在 main.c 中通过 HAL_UART_Receive_IT 使用
 */
static uint8_t s_rx_byte = 0U;

/* 这个串口句柄由 CubeMX 生成，按你的实际串口修改 */
extern UART_HandleTypeDef huart3;

void proto_rx_init(void)
{
    memset(s_rx_buf, 0, sizeof(s_rx_buf));
    s_rx_len = 0U;
    s_rx_byte = 0U;
}

void proto_rx_start(void)
{
    /*
     * 启动 1 字节中断接收
     * 每收 1 字节，HAL_UART_RxCpltCallback 会被调用一次
     */
    HAL_UART_Receive_IT(&huart3, &s_rx_byte, 1U);
}

void proto_rx_feed_byte(uint8_t byte)
{
    proto_msg_t msg;
    uint16_t expected_frame_len = 0U;
    int ret;

    /*
     * 防止缓冲区溢出
     * 如果超了，直接清空重新开始
     */
    if (s_rx_len >= PROTO_MAX_FRAME_LEN)
    {
        s_rx_len = 0U;
    }

    /* 先存入缓冲区 */
    s_rx_buf[s_rx_len++] = byte;

    /*
     * 帧同步：如果第 1 字节不是帧头 0x55，直接丢弃
     */
    if (s_rx_len == 1U)
    {
        if (s_rx_buf[0] != PROTO_HEADER1)
        {
            s_rx_len = 0U;
            return;
        }
    }

    /*
     * 帧同步：如果第 2 字节不是帧头 0xAA，也清空
     */
    if (s_rx_len == 2U)
    {
        if (s_rx_buf[1] != PROTO_HEADER2)
        {
            s_rx_len = 0U;
            return;
        }
    }

    /*
     * 收到第 3 字节后，就可以知道 payload 长度了
     * 总帧长 = payload_len + 6
     */
    if (s_rx_len >= 3U)
    {
        expected_frame_len = (uint16_t)(s_rx_buf[2] + PROTO_FRAME_OVERHEAD);

        /*
         * 如果长度异常，直接清空
         */
        if ((expected_frame_len < PROTO_FRAME_OVERHEAD) ||
            (expected_frame_len > PROTO_MAX_FRAME_LEN))
        {
            s_rx_len = 0U;
            return;
        }

        /*
         * 当收到完整一帧后，开始解包
         */
        if (s_rx_len == expected_frame_len)
        {
            memset(&msg, 0, sizeof(msg));

            ret = proto_frame_unpack(s_rx_buf, s_rx_len, &msg);
            if (ret == RET_OK)
            {
                /* 解包成功，分发命令 */
                (void)proto_dispatch_handle(&msg);
            }

            /* 无论成功失败，都清空缓冲区，准备接收下一帧 */
            s_rx_len = 0U;
        }
    }
}

void proto_rx_uart_cplt_callback(UART_HandleTypeDef *huart)
{
    if (huart == 0U)
    {
        return;
    }

    if (huart->Instance == USART3)
    {
        /* 把收到的这个字节送入协议接收处理 */
        proto_rx_feed_byte(s_rx_byte);

        /* 继续启动下一字节接收 */
        HAL_UART_Receive_IT(&huart3, &s_rx_byte, 1U);
    }
}
