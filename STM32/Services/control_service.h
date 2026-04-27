#ifndef CONTROL_SERVICE_H
#define CONTROL_SERVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  控制服务初始化
 * @return 0=成功, 负数=失败
 */
int control_service_init(void);

/*
 * @brief  设置 LED 状态
 * @param  onoff: 0=灭, 非0=亮
 * @return 0=成功, 负数=失败
 */
int control_service_set_led(uint8_t onoff);

/*
 * @brief  翻转 LED
 * @return 0=成功, 负数=失败
 */
int control_service_toggle_led(void);

/* 新增：是否允许远程控制执行器 */
int control_service_remote_action_allowed(void);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_SERVICE_H */
