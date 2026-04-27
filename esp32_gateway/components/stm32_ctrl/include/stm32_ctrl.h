#ifndef STM32_CTRL_H
#define STM32_CTRL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int stm32_ctrl_init(void);
int stm32_ctrl_send_led_set(uint8_t action);

/* 新增：设置 STM32 上报周期 */
int stm32_ctrl_send_upload_period(uint16_t period_ms);
int stm32_ctrl_send_temp_high_th(uint16_t temp_th);
int stm32_ctrl_send_light_low_th(uint16_t light_th);

/* 新增：设置工作模式 */
int stm32_ctrl_send_work_mode(uint16_t mode);

#ifdef __cplusplus
}
#endif

#endif /* STM32_CTRL_H */
