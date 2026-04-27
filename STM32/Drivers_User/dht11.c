#include "dht11.h"
#include "main.h"
#include "common_def.h"

/*
 * ===============================
 * 根据你的实际接线修改下面两个宏
 * 示例：DHT11 DATA -> PB12
 * ===============================
 */
#define DHT11_PORT          GPIOC
#define DHT11_PIN           GPIO_PIN_4

/*
 * 如果你使用的是 STM32F103，建议用一个定时器做微秒延时
 * 下面假设你已经有一个 TIM 用于 1us 计数
 * 例如：htim6 / htim2 等
 *
 * 如果你当前还没有微秒延时函数，
 * 我下面也给你一个 delay_us 的实现骨架。
 */
extern TIM_HandleTypeDef htim6;

/* 内部函数声明 */
static void dht11_set_pin_output(void);
static void dht11_set_pin_input(void);
static void dht11_delay_us(uint16_t us);
static uint8_t dht11_check_response(void);
static uint8_t dht11_read_bit(void);
static uint8_t dht11_read_byte(void);

int dht11_init(void)
{
    dht11_set_pin_output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    return RET_OK;
}

int dht11_read(dht11_data_t *data)
{
    uint8_t rh_int, rh_dec, temp_int, temp_dec, sum;
    uint8_t check_sum;

    if (data == 0U)
    {
        return RET_NULL_PTR;
    }

    /*
     * 主机发送开始信号：
     * 拉低至少 18ms，然后释放总线
     */
    dht11_set_pin_output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);   /* >= 18ms */

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    dht11_delay_us(30);

    /*
     * 切换为输入，等待 DHT11 响应
     */
    dht11_set_pin_input();

    if (!dht11_check_response())
    {
        return RET_ERR;
    }

    rh_int   = dht11_read_byte();
    rh_dec   = dht11_read_byte();
    temp_int = dht11_read_byte();
    temp_dec = dht11_read_byte();
    sum      = dht11_read_byte();

    check_sum = (uint8_t)(rh_int + rh_dec + temp_int + temp_dec);

    if (sum != check_sum)
    {
        return RET_CHECKSUM_ERR;
    }

    /*
     * DHT11 通常小数部分为 0，第一版先只取整数部分
     */
    data->humidity_int    = rh_int;
    data->temperature_int = temp_int;

    return RET_OK;
}

static uint8_t dht11_check_response(void)
{
    uint16_t timeout = 0;

    /*
     * DHT11 响应：
     * 先拉低 ~80us，再拉高 ~80us
     */

    /* 等待变低 */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        dht11_delay_us(1);
        timeout++;
        if (timeout > 100)
        {
            return 0;
        }
    }

    /* 等待变高 */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
    {
        dht11_delay_us(1);
        timeout++;
        if (timeout > 100)
        {
            return 0;
        }
    }

    /* 等待高电平结束 */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        dht11_delay_us(1);
        timeout++;
        if (timeout > 100)
        {
            return 0;
        }
    }

    return 1;
}

static uint8_t dht11_read_bit(void)
{
    uint16_t timeout = 0;

    /*
     * 每一位开始：先 50us 低电平
     */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET)
    {
        dht11_delay_us(1);
        timeout++;
        if (timeout > 100)
        {
            break;
        }
    }

    /*
     * 进入高电平后，延时约 40us 再读：
     * 高电平持续短 -> 0
     * 高电平持续长 -> 1
     */
    dht11_delay_us(40);

    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        /* 等待这一位结束 */
        timeout = 0;
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
        {
            dht11_delay_us(1);
            timeout++;
            if (timeout > 100)
            {
                break;
            }
        }
        return 1U;
    }
    else
    {
        return 0U;
    }
}

static uint8_t dht11_read_byte(void)
{
    uint8_t i;
    uint8_t byte = 0U;

    for (i = 0; i < 8; i++)
    {
        byte <<= 1;
        byte |= dht11_read_bit();
    }

    return byte;
}

static void dht11_set_pin_output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = DHT11_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;   /* 开漏输出更稳 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void dht11_set_pin_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin  = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/*
 * 1us 延时函数
 * 这里假设 htim2 已经配置为 1MHz 计数频率
 */
static void dht11_delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    HAL_TIM_Base_Start(&htim6);
    while (__HAL_TIM_GET_COUNTER(&htim6) < us)
    {
    }
    HAL_TIM_Base_Stop(&htim6);
}
