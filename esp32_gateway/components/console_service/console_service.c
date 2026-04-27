/**
 * @file console_service.c
 * @brief 控制台命令服务
 * 
 * 提供交互式命令行界面，支持设备状态查询、参数设置、LED控制等功能。
 * 支持的命令包括：help、status、ack、led控制、参数设置（周期、温度阈值、光照阈值、工作模式）。
 */

#include "console_service.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "common_def.h"
#include "project_config.h"
#include "stm32_ctrl.h"
#include "status_cache.h"
#include "param_cache.h"

/** @brief 控制台服务日志标签 */
static const char *TAG = TAG_CONSOLE_SERVICE;

/** @brief 控制台输入缓冲区大小（字节） */
#define CONSOLE_LINE_BUF_SIZE    128

/**
 * @brief 去除字符串首尾的空白字符（空格、制表符、回车、换行）
 * @param str 输入字符串
 * @return 指向去除空白后字符串的指针，若输入为NULL返回NULL
 */
static char *trim_spaces(char *str)
{
    char *end;

    if (str == NULL)
    {
        return NULL;
    }

    // 跳过字符串开头的空白字符
    while ((*str == ' ') || (*str == '\t') || (*str == '\r') || (*str == '\n'))
    {
        str++;
    }

    // 如果字符串为空，直接返回
    if (*str == '\0')
    {
        return str;
    }

    // 去除字符串末尾的空白字符
    end = str + strlen(str) - 1;
    while ((end > str) &&
           ((*end == ' ') || (*end == '\t') || (*end == '\r') || (*end == '\n')))
    {
        *end = '\0';
        end--;
    }

    return str;
}

/**
 * @brief 将ACK结果码转换为字符串描述
 * @param result ACK结果码（0x00=OK, 0x01=参数错误, 0x02=不支持, 0x03=执行错误）
 * @return 结果码的字符串表示
 */
static const char *ack_result_to_str(uint8_t result)
{
    switch (result)
    {
        case 0x00: return "OK";
        case 0x01: return "PARAM_ERR";
        case 0x02: return "UNSUPPORTED";
        case 0x03: return "EXEC_ERR";
        default:   return "UNKNOWN";
    }
}

/**
 * @brief 将命令码转换为字符串描述
 * @param cmd 命令码（0x01=数据上报, 0x02=心跳, 0x03=LED设置, 0x04=参数设置, 0x05=ACK）
 * @return 命令码的字符串表示
 */
static const char *cmd_to_str(uint8_t cmd)
{
    switch (cmd)
    {
        case 0x01: return "REPORT";
        case 0x02: return "HEARTBEAT";
        case 0x03: return "LED_SET";
        case 0x04: return "PARAM_SET";
        case 0x05: return "ACK";
        default:   return "UNKNOWN";
    }
}

/**
 * @brief 解析工作模式字符串并转换为对应的模式码
 * @param mode_str 模式字符串（"manual"=手动模式, "auto"=自动模式, "remote"=远程模式）
 * @param mode 输出参数，存储转换后的模式码（0=手动, 1=自动, 2=远程）
 * @return RET_OK 解析成功，RET_INVALID_PARAM 参数无效
 */
static int console_parse_mode(const char *mode_str, uint16_t *mode)
{
    if ((mode_str == NULL) || (mode == NULL))
    {
        return RET_INVALID_PARAM;
    }

    if (strcmp(mode_str, "manual") == 0)
    {
        *mode = 0U;
        return RET_OK;
    }
    else if (strcmp(mode_str, "auto") == 0)
    {
        *mode = 1U;
        return RET_OK;
    }
    else if (strcmp(mode_str, "remote") == 0)
    {
        *mode = 2U;
        return RET_OK;
    }

    return RET_INVALID_PARAM;
}

/**
 * @brief 打印控制台支持的命令列表帮助信息
 */
static void console_print_help(void)
{
    printf("supported commands:\r\n");
    printf("  help\r\n");
    printf("  status\r\n");
    printf("  ack\r\n");
    printf("  led on\r\n");
    printf("  led off\r\n");
    printf("  led toggle\r\n");
    printf("  set period <ms>\r\n");
    printf("  set temp_th <degC>\r\n");
    printf("  set light_th <adc>\r\n");
    printf("  set mode <manual|auto|remote>\r\n");
}

/**
 * @brief 打印当前设备状态和参数信息
 */
static void console_print_status(void)
{
    // 从缓存获取设备状态和参数
    device_status_cache_t *st = status_cache_get_device();
    device_param_cache_t *param = param_cache_get();

    // 检查缓存有效性
    if (st == NULL)
    {
        printf("status cache is null\r\n");
        return;
    }

    printf("device status:\r\n");
    printf("  temperature    : %.1f C\r\n", st->temperature);
    printf("  humidity       : %.1f %%\r\n", st->humidity);
    printf("  light          : %u\r\n", st->light);
    printf("  voltage        : %.2f V\r\n", st->voltage);
    printf("  led_state      : %u\r\n", st->led_state);
    printf("  relay_state    : %u\r\n", st->relay_state);
    printf("  buzzer_state   : %u\r\n", st->buzzer_state);
    printf("  alarm_code     : %u\r\n", st->alarm_code);
    printf("  link_state     : %u\r\n", st->link_state);
    printf("  tick_ms        : %lu\r\n", (unsigned long)st->tick_ms);
    printf("  last_update_ms : %lu\r\n", (unsigned long)st->last_update_ms);

    printf("device param:\r\n");
    printf("  upload_period_ms : %u\r\n", param->upload_period_ms);
    printf("  temp_high_th     : %u\r\n", param->temp_high_th);
    printf("  light_low_th     : %u\r\n", param->light_low_th);
    printf("  work_mode        : %u\r\n", param->work_mode);
}

/**
 * @brief 打印最后一次ACK（确认）信息
 */
static void console_print_ack(void)
{
    // 从缓存获取ACK信息
    ack_info_cache_t *ack = status_cache_get_ack();

    // 检查缓存有效性
    if (ack == NULL)
    {
        printf("ack cache is null\r\n");
        return;
    }

    printf("ack info:\r\n");
    printf("  ack_cmd        : %u (%s)\r\n", ack->ack_cmd, cmd_to_str(ack->ack_cmd));
    printf("  ack_seq        : %u\r\n", ack->ack_seq);
    printf("  ack_result     : %u (%s)\r\n", ack->ack_result, ack_result_to_str(ack->ack_result));
    printf("  last_update_ms : %lu\r\n", (unsigned long)ack->last_update_ms);
}

/**
 * @brief 执行"set"命令，设置设备参数
 * @param arg1 参数名称（"period", "temp_th", "light_th", "mode"）
 * @param arg2 参数值
 * @return RET_OK 设置成功，RET_INVALID_PARAM 参数错误
 */
static int console_execute_set(char *arg1, char *arg2)
{
    long value;
    int ret;
    uint16_t mode;

    // 参数检查
    if ((arg1 == NULL) || (arg2 == NULL))
    {
        printf("usage:\r\n");
        printf("  set period <ms>\r\n");
        printf("  set temp_th <degC>\r\n");
        printf("  set light_th <adc>\r\n");
        printf("  set mode <manual|auto|remote>\r\n");
        return RET_INVALID_PARAM;
    }

    // 处理工作模式设置
    if (strcmp(arg1, "mode") == 0)
    {
        ret = console_parse_mode(arg2, &mode);
        if (ret != RET_OK)
        {
            printf("mode must be: manual | auto | remote\r\n");
            return RET_INVALID_PARAM;
        }

        // 发送模式设置命令到STM32，成功时更新参数缓存
        ret = stm32_ctrl_send_work_mode(mode);
        if (ret == RET_OK)
        {
            param_cache_set_work_mode(mode);
        }
        return ret;
    }

    // 对于其他参数，将字符串转换为数值
    value = strtol(arg2, NULL, 10);

    // 设置上传周期参数（100-10000ms）
    if (strcmp(arg1, "period") == 0)
    {
        if ((value < 100) || (value > 10000))
        {
            printf("period out of range: 100 ~ 10000 ms\r\n");
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_upload_period((uint16_t)value);
        if (ret == RET_OK)
        {
            param_cache_set_upload_period((uint16_t)value);
        }
        return ret;
    }
    // 设置温度阈值参数（0-80°C）
    else if (strcmp(arg1, "temp_th") == 0)
    {
        if ((value < 0) || (value > 80))
        {
            printf("temp_th out of range: 0 ~ 80 degC\r\n");
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_temp_high_th((uint16_t)value);
        if (ret == RET_OK)
        {
            param_cache_set_temp_high_th((uint16_t)value);
        }
        return ret;
    }
    // 设置光照阈值参数（0-4095）
    else if (strcmp(arg1, "light_th") == 0)
    {
        if ((value < 0) || (value > 4095))
        {
            printf("light_th out of range: 0 ~ 4095\r\n");
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_light_low_th((uint16_t)value);
        if (ret == RET_OK)
        {
            param_cache_set_light_low_th((uint16_t)value);
        }
        return ret;
    }

    printf("usage:\r\n");
    printf("  set period <ms>\r\n");
    printf("  set temp_th <degC>\r\n");
    printf("  set light_th <adc>\r\n");
    printf("  set mode <manual|auto|remote>\r\n");
    return RET_INVALID_PARAM;
}

/**
 * @brief 执行单行命令
 * @param line 输入的命令行字符串
 * @return RET_OK 执行成功，RET_INVALID_PARAM 命令格式错误
 */
static int console_execute_line(char *line)
{
    char *cmd = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL;

    // 去除首尾空白字符
    line = trim_spaces(line);
    if ((line == NULL) || (*line == '\0'))
    {
        return RET_OK;
    }

    // 提取命令部分
    cmd = strtok(line, " ");
    if (cmd == NULL)
    {
        return RET_OK;
    }

    // 处理各个命令
    if (strcmp(cmd, "help") == 0)
    {
        console_print_help();
        return RET_OK;
    }

    if (strcmp(cmd, "status") == 0)
    {
        console_print_status();
        return RET_OK;
    }

    if (strcmp(cmd, "ack") == 0)
    {
        console_print_ack();
        return RET_OK;
    }

    // LED控制命令（on/off/toggle）
    if (strcmp(cmd, "led") == 0)
    {
        arg1 = strtok(NULL, " ");
        if (arg1 == NULL)
        {
            printf("usage: led on|off|toggle\r\n");
            return RET_INVALID_PARAM;
        }

        // 根据参数发送对应的LED控制命令
        if (strcmp(arg1, "on") == 0)
        {
            return stm32_ctrl_send_led_set(1U);
        }
        else if (strcmp(arg1, "off") == 0)
        {
            return stm32_ctrl_send_led_set(0U);
        }
        else if (strcmp(arg1, "toggle") == 0)
        {
            return stm32_ctrl_send_led_set(2U);
        }
        else
        {
            printf("usage: led on|off|toggle\r\n");
            return RET_INVALID_PARAM;
        }
    }

    // 参数设置命令
    if (strcmp(cmd, "set") == 0)
    {
        arg1 = strtok(NULL, " ");
        arg2 = strtok(NULL, " ");
        return console_execute_set(arg1, arg2);
    }

    printf("unknown command: %s\r\n", cmd);
    printf("type 'help'\r\n");
    return RET_INVALID_PARAM;
}

/**
 * @brief 初始化控制台服务
 * @return RET_OK 初始化成功
 */
int console_service_init(void)
{
    ESP_LOGI(TAG, "console_service_init");
    return RET_OK;
}

/**
 * @brief 控制台服务任务（FreeRTOS任务函数）
 * 
 * 该任务运行一个持续的控制台循环，接收用户输入的命令并执行。
 * 命令行输入通过stdin，输出通过stdout到控制台。
 * 
 * @param pvParameters 任务参数（未使用）
 */
void console_service_task(void *pvParameters)
{
    char line_buf[CONSOLE_LINE_BUF_SIZE];
    int ret;
    int prompt_printed = 0;

    (void)pvParameters;

    printf("\r\nsimple console started\r\n");
    printf("type 'help'\r\n");

    while (1)
    {
        // 打印提示符（仅在需要时打印）
        if (!prompt_printed)
        {
            printf("esp32> ");
            fflush(stdout);
            prompt_printed = 1;
        }

        // 从stdin读取用户输入
        if (fgets(line_buf, sizeof(line_buf), stdin) == NULL)
        {
            // 读取失败，短暂延迟后重试
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        prompt_printed = 0;

        // 执行读取到的命令行
        ret = console_execute_line(line_buf);
        if (ret == RET_OK)
        {
            printf("OK\r\n");
        }
        else
        {
            printf("ERR=%d\r\n", ret);
        }
    }
}
