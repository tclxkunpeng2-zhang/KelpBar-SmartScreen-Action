/* BLE Mesh 通信模块头文件
 * 提供主机对 BLE Mesh 设备（RGB灰、开关、传感器）的控制接口。
 * 通信方式：通过串口发送/接收 AT 指令与 BLE 模组交互。
 */
#ifndef _BLE_MESH_H_
#define _BLE_MESH_H_

#include "device_data.h"

/* 向 BLE 模组发送自定义串口数据包 */
void send_tt_cmd(uint8_t *cmd,uint8_t len);

/* 控制 RGB LED 三通道开关状态及 Mesh 开关状态 */
void ctrl_rgb_and_switch(uint8_t r,uint8_t g, uint8_t b,uint8_t switch_state);

/* 控制自定义广播扫描开关（用于接收传感器上报的广播数据） */
void scan_adv_ctl(bool state);

/* 初始化 BLE Mesh：打开串口、配置主机名称和居失列表，并启动接收线程 */
void ble_mesh_init(void);

#endif

