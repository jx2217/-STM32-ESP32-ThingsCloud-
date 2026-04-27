#ifndef LED_H
#define LED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LED_GPIO_Port  GPIOB
#define LED_Pin        GPIO_PIN_0

/*
 * @brief  LED 模块初始化
 * @note   如果 GPIO 已经由 CubeMX 初始化完成，这里可以为空实现
 */
void led_init(void);

/*
 * @brief  设置 LED 状态
 * @param  onoff: 0=灭, 非0=亮
 */
void led_set(uint8_t onoff);

/*
 * @brief  翻转 LED 状态
 */
void led_toggle(void);

/*
 * @brief  获取 LED 当前状态
 * @return 0=灭, 1=亮
 */
uint8_t led_get(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
