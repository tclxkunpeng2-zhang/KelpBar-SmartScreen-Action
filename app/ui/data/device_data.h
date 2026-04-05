/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 18:24:33 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 01:14:51
 */

/* 设备全局状态数据模型
 * 定义整个应用的核心状态结构体 device_state_t 及相关枚举类型，
 * 供各 UI 页面和后台模块读写设备状态。
 */
#ifndef _DEVICE_DATA_H_
#define _DEVICE_DATA_H_

#include <stdio.h>
#include "lvgl.h"
#include "wpa_manager.h"

/* 天气查询默认城市及 API Key（心知天气服务） */
#define WEATHER_CITY "guangzhou"
#define WEATHER_KEY "SmazqPcltzTft-X3v"

/* 开关通用状态 */
typedef enum{
    OFF = 0,    /* 关 */
    ON,         /* 开 */
}SWITCH_STATE_E;

/* BLE/设备连接状态 */
typedef enum{
    DISCONNECT = 0, /* 未连接 */
    CONNECTING,     /* 连接中 */
    CONNECT,        /* 已连接 */
}CONNECT_STATE_E;

/* 主页表盘显示类型 */
typedef enum{
    TIME_TYPE_1 = 0,    /* 数字时间 + 天气（大号字体） */
    TIME_TYPE_2,        /* 数字时间（数字字体居中） */
    TIME_TYPE_3,        /* 模拟指针时钟 */
}TIME_SHOW_TYPE_E;

/* 告警通知方式 */
typedef enum{
    NOTICE_TYPE_ONLY_WIN = 0,       /* 仅弹窗提醒 */
    NOTICE_TYPE_WIN_AND_AUDIO,      /* 弹窗 + 声音提醒 */
}NOTICE_TYPE_E;

/* 设备全局状态结构体，整个运行期间只有一个实例 */
typedef struct{

    CONNECT_STATE_E ble_mesh_state;             /* BLE Mesh 模组连接状态 */
    CONNECT_STATE_E led_connect_state;          /* RGB LED 灯连接状态 */
    CONNECT_STATE_E switch_connect_state;       /* Mesh 开关连接状态 */
    CONNECT_STATE_E body_sensor_connect_state;  /* 人体感应设备连接状态 */
    CONNECT_STATE_E smart_coaster_connect_state;/* 智能杯垫连接状态 */
    CONNECT_STATE_E flame_sensor_connect_state; /* 火焰传感器连接状态 */

    /* 各设备电量（0-100%） */
    uint8_t rgbled_battery_level;
    uint8_t switch_battery_level;
    uint8_t body_sensor_battery_level;
    uint8_t smart_coaster_battery_level;
    uint8_t flame_sensor_battery_level;

    /* RGB LED 三通道开关状态 */
    SWITCH_STATE_E led_r;
    SWITCH_STATE_E led_g;
    SWITCH_STATE_E led_b;

    SWITCH_STATE_E mesh_switch_state;   /* Mesh 开关状态（ON/OFF） */

    /* 人体感应相关 */
    SWITCH_STATE_E body_sensor_state;           /* 人体感应功能开关 */
    uint8_t body_sensor_data;                   /* 人体感应当前检测值（0=无人，1=有人） */
    NOTICE_TYPE_E body_sensor_notice_type;      /* 人体感应触发时的通知方式 */

    /* 智能杯垫相关 */
    SWITCH_STATE_E smart_coaster_state;         /* 智能杯垫功能开关 */
    uint8_t smart_coaster_data;                 /* 杯垫当前状态值 */
    uint32_t smart_coaster_time;                /* 喝水提醒间隔时长（单位：min） */
    uint32_t smart_coaster_remaining_time;      /* 距下次提醒剩余时间（单位：min） */

    /* 火焰传感器相关 */
    SWITCH_STATE_E flame_sensor_state;          /* 火焰监测功能开关 */
    uint8_t flame_sensor_data;                  /* 火焰传感器当前状态值 */
    uint32_t flame_sensor_time;                 /* 关火提醒间隔时长（单位：min） */
    uint32_t flame_sensor_remaining_time;       /* 距关火提醒剩余时间（单位：min） */

    /* 番茄钟相关 */
    SWITCH_STATE_E tomato_time_state;               /* 番茄钟计时开关（运行/暂停） */
    uint32_t tomato_learn_time;                     /* 设置的学习时长（min） */
    uint32_t tomato_learn_remaining_time;           /* 当前学习剩余时间（min） */
    uint32_t tomato_rest_time;                      /* 设置的休息时长（min） */
    uint32_t tomato_rest_remaining_time;            /* 当前休息剩余时间（min） */

    char weather_info[50];      /* 当前天气信息字符串（如 "广州 多云 26°C"） */
    char weather_city[20];      /* 当前选中的天气城市名/ID */

    TIME_SHOW_TYPE_E time_type; /* 主页表盘显示类型 */
    int clock_type;             /* 指针表盘样式（0/1/2 三种样式） */

    WPA_WIFI_STATUS_E wifi_state;               /* WiFi 功能开启状态 */
    WPA_WIFI_CONNECT_STATUS_E wifi_connect_state; /* WiFi 连接状态 */

    int brightness_value;   /* 屏幕亮度值（0-100） */
    int volume_value;       /* 音量值（0-100） */

    /* 闹钟1：倒计时模式（N分钟后提醒） */
    bool is_open_type1;     /* 闹钟1是否已启用 */
    int alarm_time1;        /* 闹钟1剩余分钟数 */

    /* 闹钟2：定时模式（到达指定时刻提醒） */
    bool is_open_type2;     /* 闹钟2是否已启用 */
    int alarm_time2;        /* 闹钟2触发时刻（hour*60+min） */

    bool is_disp_orientation; /* 是否旋转屏幕方向（横屏模式） */

}device_state_t;

/* 函数声明 */
void init_device_state(void);           /* 初始化设备状态（从文件读取持久化参数） */
void device_timer_init(void);           /* 初始化1秒定时器（驱动倒计时等逻辑） */
void device_param_write(void);          /* 将设备状态持久化写入文件 */
void device_param_read(void);           /* 从文件读取设备状态参数 */
void update_wpa_manager_callback_fun(void); /* 更新 WiFi 状态回调注册 */
device_state_t* get_device_state(void); /* 获取全局设备状态指针 */

#endif
