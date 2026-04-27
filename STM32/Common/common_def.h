#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 通用返回值 */
#define RET_OK              (0)
#define RET_ERR             (-1)
#define RET_NULL_PTR        (-2)
#define RET_INVALID_PARAM   (-3)
#define RET_CHECKSUM_ERR    (-4)
#define RET_PROTO_ERR       (-5)

/* 工作模式 */
typedef enum
{
    MODE_MANUAL = 0,
    MODE_AUTO,
    MODE_REMOTE
} work_mode_t;

/* 系统状态 */
typedef enum
{
    SYS_INIT = 0,
    SYS_IDLE,
    SYS_RUN,
    SYS_ALARM,
    SYS_FAULT
} sys_state_t;

/* 告警码 */
typedef enum
{
    ALARM_NONE = 0,       /* 无告警 */
    ALARM_SENSOR_ERR,     /* 传感器异常 */
    ALARM_LINK_LOST,      /* 链路丢失 */
    ALARM_VOLT_LOW,       /* 电压过低 */

    ALARM_TEMP_HIGH,      /* 温度过高 */
    ALARM_LIGHT_LOW       /* 光照过低 */
} alarm_code_t;

/* 链路状态 */
typedef enum
{
    LINK_OFFLINE = 0,
    LINK_ONLINE  = 1
} link_state_t;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_DEF_H */
