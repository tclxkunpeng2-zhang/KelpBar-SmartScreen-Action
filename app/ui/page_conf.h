/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:06 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:24:44
 */

/* 页面配置头文件
 * 定义所有页面枚举类型及初始化函数声明，供各页面模块相互跳转调用。
 */
#ifndef _PAGE_CONF_H_
#define _PAGE_CONF_H_

#include "lvgl.h"
#include "music_conf.h"
#include "em_hal_audio.h"
#include "audio_player_async.h"

/* 联动设置参数类型：人体感应 / 智能杯垫 / 火焰传感器 */
typedef enum{   
    TYPE_BODY_SENSOR = 0,   /* 人体感应 */
    TYPE_SMART_COASTER,     /* 智能杯垫 */
    TYPE_FLAME_SENSOR,      /* 火焰传感器 */
}SETTING_PARAM_TYPE_E;

/* 告警类型：人体感应触发 / 杯垫提醒触发 / 火焰告警触发 */
typedef enum{
    WARN_BODY_SENSOR_TRIGGER = 0,   /* 人体感应告警 */
    WARN_SMART_COASTER_TRIGGER,     /* 智能杯垫提醒告警 */
    WARN_FLAME_SENSOR_TRIGGER,      /* 火焰传感器告警 */
}WRANING_TYPE_E;

/* 闹钟触发类型 */
typedef enum{
    ALARM_ALARM_CLOCK_TRIGGER = 0,  /* 普通闹钟触发 */
}ALARM_TYPE_E;

/* 删除当前活动页面并重置样式 */
void delete_current_page(lv_style_t *style);

/* 各页面初始化函数声明 */
void init_page_main(void);                                      /* 主页面（表盘/菜单） */
void init_page_linkage(void);                                   /* 场景联动页面 */
void init_page_linkage_setting(SETTING_PARAM_TYPE_E type);      /* 联动设备详细设置页面 */
void init_page_warn(WRANING_TYPE_E type);                       /* 告警弹窗页面 */
void init_page_wifi_setting(void);                              /* WiFi 设置页面 */
void init_page_city_setting(void);                              /* 城市选择页面 */
void init_page_time_setting(void);                              /* 表盘样式设置页面 */
void init_page_tomatotime_setting(void);                        /* 番茄钟时长设置页面 */
void init_page_tomato_learn(void);                              /* 番茄钟-学习倒计时页面 */
void init_page_tomato_rest(void);                               /* 番茄钟-休息倒计时页面 */
void init_page_tomato_finish(void);                             /* 番茄钟-本轮完成页面 */
void init_page_system_setting(void);                            /* 系统设置页面（亮度/音量/方向） */
void init_page_about_us(void);                                  /* 关于页面 */
void init_page_pulldowm_view(lv_obj_t *view);                   /* 下拉控制栏（挂载到指定父对象） */
void deinit_page_pulldowm_view(void);                           /* 销毁下拉控制栏 */
void init_page_sleep(void);                                     /* 息屏/睡眠页面（60s无操作后触发） */
bool get_is_pull_down(void);                                    /* 获取下拉栏当前展开状态 */
void init_page_connect_loading(const char *name,const char *psw); /* WiFi 连接等待页面 */
void init_page_alarm_clock_setting(void);                       /* 闹钟设置页面 */
void init_page_alarm_remind(ALARM_TYPE_E type);                 /* 闹钟提醒弹窗 */
void init_page_shortcutkey(void);                               /* 快捷键（USB HID）页面 */
void init_page_language_game(void);                             /* 语音游戏页面 */
void init_page_simulate_iphone(void);                           /* iPhone 界面模拟页面 */

#endif
