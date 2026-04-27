#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include "device_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief  存储服务初始化
 * @note   内部可为空实现，主要保留统一入口
 */
int storage_service_init(void);

/*
 * @brief  从 Flash 加载参数到 device_data
 * @return 0=成功, 负数=失败
 */
int storage_service_load_param(void);

/*
 * @brief  将当前参数保存到 Flash
 * @return 0=成功, 负数=失败
 */
int storage_service_save_param(void);

/*
 * @brief  恢复默认参数并保存到 Flash
 * @return 0=成功, 负数=失败
 */
int storage_service_reset_param_to_default(void);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_SERVICE_H */
