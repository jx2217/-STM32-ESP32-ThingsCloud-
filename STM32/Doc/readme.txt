┌──────────────────────────────────────────────┐
│                  应用层 App                  │
│                                              │
│ main.c      app_main.c                       │
│ 负责启动      负责总调度                     │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│               业务层 Services                │
│                                              │
│ acquire_service.c                            │
│ report_service.c                             │
│ control_service.c                            │
│                                              │
│ 负责“要做什么业务”                           │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│               协议层 Protocol                │
│                                              │
│ proto_cmd.h                                  │
│ proto_msg.h                                  │
│ proto_frame.c                                │
│ proto_rx.c                                   │
│ proto_dispatch.c                             │
│                                              │
│ 负责“怎么通信、怎么翻译协议”                 │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│             驱动层 Drivers_User              │
│                                              │
│ led.c                                        │
│                                              │
│ 负责“具体硬件动作”                           │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│                 BSP / HAL                    │
│                                              │
│ bsp_uart.c                                   │
│ HAL_GPIO_WritePin()                          │
│ HAL_UART_Transmit()                          │
│                                              │
│ 负责“最底层硬件接口调用”                     │
└──────────────────────────────────────────────┘


┌──────────────────────────────────────────────┐
│                公共支撑层 Common             │
│                                              │
│ common_def.h                                 │
│ common_crc.c                                 │
│ device_data.c                                │
│                                              │
│ 给上面各层提供公共能力和共享数据             │
└──────────────────────────────────────────────┘







| Byte0 | Byte1 | Byte2 | Byte3 | Byte4 | Byte5... | LastByte |
|  H1   |  H2   |  LEN  |  CMD  |  SEQ  | PAYLOAD  | CHECKSUM |

| 帧头1 | 帧头2 | 数据长度 | 命令字 | 序号 | 数据区 | 校验 |

CMD：
CMD_REPORT    = 0x01   //状态上报  STM32 主动发给外部，表示“这是我的当前状态数据”
CMD_HEARTBEAT = 0x02   //心跳   一般用于链路在线检测
CMD_LED_SET   = 0x03   //LED 控制命令    外部发给 STM32，用于控制 LED
CMD_PARAM_SET = 0x04   //参数下发    外部发给 STM32，用于修改参数
CMD_ACK       = 0x05   //ACK   STM32 回给外部，用于表示某个命令执行结果


55 AA LEN 01 SEQ PAYLOAD CHECKSUM    //状态上报帧
Byte0~1   temp_x10      int16
Byte2~3   humi_x10      uint16
Byte4~5   light         uint16
Byte6~7   volt_x100     uint16
Byte8     led_state     uint8
Byte9     relay_state   uint8
Byte10    buzzer_state  uint8
Byte11    alarm_code    uint8
Byte12    link_state    uint8      //表示 STM32 认为和外部链路是否在线。  0x00 = 离线    0x01 = 在线
Byte13    reserved      uint8      //保留
Byte14~17 tick_ms       uint32     //系统运行时间

alarm_code:
ALARM_NONE       = 0   //无警告
ALARM_SENSOR_ERR = 1   //传感器异常
ALARM_LINK_LOST  = 2   //链路丢失
ALARM_VOLT_LOW   = 3   //电压低



ACK：
55 AA LEN 05 SEQ PAYLOAD CHECKSUM   //这里 CMD = 05，说明这是 ACK 帧。

SEQ：这条 ACK 自己的序号

PAYLOAD格式：
Byte0  ack_cmd    //表示这条 ACK 是在应答哪一种命令。
Byte1  ack_seq    //表示这条 ACK 是在应答哪一个序号的命令。
Byte2  result     //表示执行结果。00 成功   01 参数错误    02 不支持的命令   03 执行失败


