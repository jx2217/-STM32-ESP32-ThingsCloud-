#ifndef CONSOLE_SERVICE_H
#define CONSOLE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  控制台服务初始化
 * @return 0=成功, 负数=失败
 */
int console_service_init(void);

/*
 * @brief  控制台任务入口
 * @param  pvParameters: FreeRTOS 参数
 */
void console_service_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_SERVICE_H */
