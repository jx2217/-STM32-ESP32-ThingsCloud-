# esp32_gateway

First version target:

- Receive STM32 REPORT frame over UART
- Receive STM32 ACK frame over UART
- Parse protocol frame
- Print parsed result with ESP_LOGI



1.mqtt推送任务    mqtt_service.c       void mqtt_publish_task(void *pvParameters)   //状态改变或者到了心跳周期就上报一次。

2.控制台任务      console_service.c    void console_service_task(void *pvParameters) //读取控制命令，根据命令控制STM32动作或修改参数（调用STM32_ctrl.c）

3.串口接收任务    uart_service.c       void uart_rx_task(void *pvParameters)  //接收完STM32的数据之后存入缓存区