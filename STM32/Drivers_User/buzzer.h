#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  蜂鸣器初始化
 */
void buzzer_init(void);

/*
 * @brief  设置蜂鸣器状态
 * @param  on: 0=关闭, 1=打开
 */
void buzzer_set(uint8_t on);

/*
 * @brief  打开蜂鸣器
 */
void buzzer_on(void);

/*
 * @brief  关闭蜂鸣器
 */
void buzzer_off(void);

/*
 * @brief  翻转蜂鸣器状态
 */
void buzzer_toggle(void);

/*
 * @brief  获取当前蜂鸣器逻辑状态
 * @return 0=关闭, 1=打开
 */
uint8_t buzzer_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H */
