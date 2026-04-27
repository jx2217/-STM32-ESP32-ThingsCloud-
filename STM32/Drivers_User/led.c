#include "led.h"
#include "main.h"

/* 
 * 用静态变量保存 LED 当前状态
 * 这样即使外部没有读 GPIO，也能直接知道逻辑状态
 */
static uint8_t s_led_state = 0U;

void led_init(void)
{
    /* 
     * 如果 GPIO 已经在 MX_GPIO_Init() 中初始化好了，
     * 这里可以不做任何操作。
     * 为了安全，初始化时先关闭 LED。
     */
    led_set(0U);
}

void led_set(uint8_t onoff)
{
    if (onoff)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        s_led_state = 1U;
    }
    else
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        s_led_state = 0U;
    }
}

void led_toggle(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

    /* 同步更新软件状态 */
    s_led_state = (s_led_state == 0U) ? 1U : 0U;
}

uint8_t led_get(void)
{
    return s_led_state;
}
