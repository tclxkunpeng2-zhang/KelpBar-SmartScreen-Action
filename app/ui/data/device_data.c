/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 18:56:40 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 01:14:26
 */

/* 设备全局状态管理实现
 * 负责设备状态初始化、持久化读写、1秒定时器逻辑
 * 以及 WiFi/天气回调注册等功能。
 */
#include "device_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "file_save.h"
#include "http_manager.h"
#include "em_hal_audio.h"
#include "em_hal_brightness.h"
#include "page_conf.h"


/* 定时器后端选择：默认使用 LVGL 软件定时器；若定义 USE_SYSTEM_TIMER 则使用系统 SIGALRM */
// #define USE_SYSTEM_TIMER 1
#ifndef USE_SYSTEM_TIMER
    #define USE_LVGL_TIMER 1
#endif

device_state_t device_state; /* 全局唯一设备状态实例 */

/* 内部秒/分/时计数器（随定时器每秒递增，用于触发倒计时逻辑） */
static int time_s = 0;
static int time_m = 0;
static int time_h = 0;

static time_t timep;
static struct tm time_temp;

/**
 * 每秒检查各联动设备是否需要触发告警弹窗：
 * - 人体感应检测到有人时触发告警
 * - 智能杯垫倒计时归零时触发喝水提醒并重置倒计时
 * - 火焰传感器倒计时归零时触发告警并重置倒计时
 */
static void warn_time_count_handle(){
    device_state_t* device_state = get_device_state();
    if(device_state->body_sensor_connect_state == CONNECT && device_state->body_sensor_state == ON &&  device_state->body_sensor_data == 1){
        init_page_warn(WARN_BODY_SENSOR_TRIGGER);
    }
    if(device_state->smart_coaster_connect_state == CONNECT && device_state->smart_coaster_state == ON &&  device_state->smart_coaster_remaining_time == 0){
        init_page_warn(WARN_SMART_COASTER_TRIGGER);
        device_state->smart_coaster_remaining_time = device_state->smart_coaster_time;
    }
    if(device_state->flame_sensor_connect_state == CONNECT && device_state->flame_sensor_state == ON &&  device_state->flame_sensor_remaining_time == 0){
        init_page_warn(WARN_FLAME_SENSOR_TRIGGER);
        device_state->flame_sensor_remaining_time = device_state->flame_sensor_time;
    }
}

/**
 * 每分钟检查闹钟是否需要触发：
 * - 类型1（倒计时模式）：alarm_time1 归零时触发
 * - 类型2（定时模式）：当前时刻等于设定时刻时触发
 */
static void alarm_time_count_min(){
    if(device_state.is_open_type1){
        if(device_state.alarm_time1 > 0){
            device_state.alarm_time1 --;
        }
        if(device_state.alarm_time1 == 0){
            //TODO alarm
            init_page_alarm_remind(ALARM_ALARM_CLOCK_TRIGGER);
            device_state.is_open_type1 = false;
        }
    }
    if(device_state.is_open_type2){
        time(&timep);
        memcpy(&time_temp, localtime(&timep), sizeof(struct tm));
        if((time_temp.tm_hour*60+time_temp.tm_min) == device_state.alarm_time2){
            //TODO alarm
            init_page_alarm_remind(ALARM_ALARM_CLOCK_TRIGGER);
            device_state.is_open_type2 = false;
        }
    }
}

/**
 * 定时器每秒回调：递增秒/分/时计数，
 * 并触发告警检查、闹钟检查、各设备倒计时递减以及番茄钟倒计时递减。
 */
static void timer_count(){
    time_s ++;
    warn_time_count_handle();
    //min
    if(time_s >= 60){
        time_m ++;
        time_s = 0;
        alarm_time_count_min();
        if(device_state.flame_sensor_state == ON){
            if(device_state.flame_sensor_remaining_time > 0){
                device_state.flame_sensor_remaining_time --;
            }
        }
        if(device_state.smart_coaster_state == ON){
            if(device_state.smart_coaster_remaining_time > 0){
                device_state.smart_coaster_remaining_time --;
            }
        }
        if(device_state.tomato_time_state == ON){
            if(device_state.tomato_learn_remaining_time > 0){
                device_state.tomato_learn_remaining_time --;
            }
            if(device_state.tomato_learn_remaining_time == 0){
                if(device_state.tomato_rest_remaining_time > 0)
                    device_state.tomato_rest_remaining_time --;
            }
        }
    }
    //hours
    if(time_m >= 60){
        time_h ++;
        time_m = 0;
    }
    //day
    if(time_h >= 24){
        time_h = 0;
    }
}

#ifdef USE_LVGL_TIMER
static void lv_timer_cb_func(lv_timer_t * timer)
{
    // printf("lv_timer_cb_func timer run ...\n");
    timer_count();
}
#elif USE_SYSTEM_TIMER
static void timer_handler(int signum) {
    static int count = 0;
    printf("Timer tick %d\n", ++count);
    timer_count();
}
#endif


/**
 * 初始化设备定时器。
 * LVGL 模式：创建 1000ms 的 lv_timer 定时器。
 * 系统模式：注册 SIGALRM 信号，使用 setitimer 设置 1s 周期定时器。
 */
void device_timer_init(){
    printf("device_timer_init\n");
#ifdef USE_LVGL_TIMER
    lv_timer_t * device_timer = lv_timer_create(lv_timer_cb_func, 1000, NULL);
#elif USE_SYSTEM_TIMER
    struct itimerval system_timer;
    // 注册信号处理函数
    signal(SIGALRM, timer_handler);
    // 初始化定时器
    system_timer.it_value.tv_sec = 1;  // 初始延迟时间，1秒
    system_timer.it_value.tv_usec = 0; // 微秒部分
    system_timer.it_interval.tv_sec = 1;  // 重复间隔时间，1秒
    system_timer.it_interval.tv_usec = 0; // 微秒部分
    // 设置定时器
    if (setitimer(ITIMER_REAL, &system_timer, NULL) == -1) {
        perror("setitimer error");
    }
#endif
}

/**
 * 将当前设备状态序列化写入持久化文件 param.cfg，
 * 使参数在重启后仍可恢复。
 */
void device_param_write(){
    int ret = file_param_write("param.cfg",&device_state,sizeof(device_state));
    if(ret == 0)
        printf("device_param_write success\n");
}

/**
 * 从持久化文件 param.cfg 读取设备状态参数，
 * 用于开机时恢复上次的配置（亮度、音量、闹钟、城市等）。
 */
void device_param_read(){
    int ret = file_param_read("param.cfg",&device_state,sizeof(device_state));
    if(ret == 0)
        printf("device_param_read success ble_mesh_state = %d \n",device_state.ble_mesh_state);
}

/* HTTP 天气数据回调：将返回的天气字符串存入设备状态 */
static void weather_callback_func(char *data){
    printf("---->%s\n",data);
    strcpy(device_state.weather_info,data);
}

/* WiFi 连接状态变化回调：更新 wifi_connect_state 字段 */
static void connect_status_callback_func(WPA_WIFI_CONNECT_STATUS_E status){
    printf("davicedata----->connect_status_callback_func: %d\n", status);
    device_state.wifi_connect_state = status;
}

/* WiFi 功能状态变化回调：更新 wifi_state 字段 */
static void wifi_status_callback_func(WPA_WIFI_STATUS_E status){
    printf("----->wifi_status_callback_func: %d\n", status);
    device_state.wifi_state = status;
}

/* 重新注册 WiFi 状态/连接回调，供页面初始化时调用 */
void update_wpa_manager_callback_fun(){
    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);
}

/**
 * 初始化全局设备状态：
 * 1. 将结构体清零
 * 2. 设置默认值（表盘类型、时钟样式、屏幕方向等）
 * 3. 从文件读取持久化参数
 * 4. 对不合理的值做保护（防止黑屏、音量异常等）
 * 5. 注册 WiFi 和天气回调
 */
void init_device_state(){

    memset(&device_state,0,sizeof(device_state_t));

    device_state.time_type = TIME_TYPE_3;
    device_state.clock_type = 2;
    device_state.is_disp_orientation = false;

    device_param_read();

    device_state.led_r = 0;
    device_state.led_g = 0;
    device_state.led_b = 0;

    device_state.led_connect_state = DISCONNECT;
    device_state.switch_connect_state = DISCONNECT;
    device_state.mesh_switch_state = OFF;
    device_state.body_sensor_state = OFF;

    device_state.smart_coaster_connect_state = DISCONNECT;
    device_state.smart_coaster_state = OFF;
    if(device_state.smart_coaster_time == 0){
        device_state.smart_coaster_time = 20;
        device_state.smart_coaster_remaining_time = 20;
    }

    device_state.body_sensor_connect_state = DISCONNECT;
    device_state.body_sensor_state = OFF;
    device_state.body_sensor_notice_type = NOTICE_TYPE_WIN_AND_AUDIO;
    
    device_state.flame_sensor_connect_state = DISCONNECT;
    device_state.flame_sensor_state = OFF;
    device_state.flame_sensor_data = 0;
    if(device_state.flame_sensor_time == 0){
        device_state.flame_sensor_time = 20;
        device_state.flame_sensor_remaining_time = 20;
    }

    device_state.switch_battery_level = 0;
    device_state.body_sensor_battery_level = 0;
    device_state.flame_sensor_battery_level = 0;
    device_state.smart_coaster_battery_level = 0;

    device_state.wifi_connect_state = WPA_WIFI_INACTIVE;
    device_state.ble_mesh_state = DISCONNECT;

    device_state.is_open_type1 = false;
    device_state.is_open_type2 = false;

    device_state.brightness_value = em_hal_brightness_get_value();
    if(device_state.brightness_value == 0){
        //避免黑屏情况
        em_hal_brightness_set_value(10);
    }
    if(device_state.volume_value != 0){
        em_set_audio_vol(device_state.volume_value);
    }else
        device_state.volume_value = em_get_audio_vol();

    if(strlen(device_state.weather_city) == 0)
        strcpy(device_state.weather_city,"guangzhou");
    if(strlen(device_state.weather_info) == 0)
        strcpy(device_state.weather_info,"广州 多云 26°C");

    

    http_set_weather_callback(weather_callback_func);
    wpa_manager_add_callback(wifi_status_callback_func,connect_status_callback_func);
}

/* 获取全局设备状态指针，所有模块通过此函数访问共享状态 */
device_state_t* get_device_state(){
    return &device_state;
}