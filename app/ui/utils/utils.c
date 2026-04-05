/*
 * @Author: xiaozhi 
 * @Date: 2024-09-25 00:07:46 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-26 02:56:32
 */

/* 通用工具函数实现
 * 提供时间格式化和系统信号初始化。
 *
 * 时间格式说明：
 *   get_time_str:          输出正常字符 "HH:MM"，用于默认显示状态
 *   get_time_str_nosymbol: 将冒号替换为黑色字符，配合 LVGL recolor 实现闪烁
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "lvgl.h"
#include "utils.h"

static char time[20];

/**
 * 将分钟数转换为 "HH:MM" 格式字符串，单位数补零。
 * 注意：返回静态缓冲区指针，非线程安全。
 */
char *get_time_str(int time_min){
    memset(time,'\0',sizeof(time));
    int time_h = time_min / 60;
    int time_m = time_min % 60;
    if(time_h >= 10){
        if(time_m >= 10){
            sprintf(time,"%d:%d",time_h,time_m);
        }else{
            sprintf(time,"%d:0%d",time_h,time_m);
        }
    }else{
        if(time_m >= 10){
            sprintf(time,"0%d:%d",time_h,time_m);
        }else{
            sprintf(time,"0%d:0%d",time_h,time_m);
        }
    }
    return time;
}   

/**
 * 将分钟数转换为 "HH:MM" 格式字符串，其中冒号（:)用黑色隐藏实现闪烁效果。
 * 需配合 lv_label_set_recolor(label, true) 使用。
 */
char *get_time_str_nosymbol(int time_min){
    memset(time,'\0',sizeof(time));
    int time_h = time_min / 60;
    int time_m = time_min % 60;
    if(time_h >= 10){
        if(time_m >= 10){
            sprintf(time,"%d#000000 :#%d",time_h,time_m);
        }else{
            sprintf(time,"%d#000000 :#0%d",time_h,time_m);
        }
    }else{
        if(time_m >= 10){
            sprintf(time,"0%d#000000 :#%d",time_h,time_m);
        }else{
            sprintf(time,"0%d#000000 :#0%d",time_h,time_m);
        }
    }
    return time;
}


/**
 * 信号回调函数：捕获常见致命信号，打印日志后调用 _exit(1) 退出。
 * 防止因捕获 SIGSEGV 等信号后无限重入导致死机。
 */
static void signal_callback_func(int sig_no)
{
    printf("signal %d, exiting ...\n", sig_no);
    _exit(1);
    printf("retry _exit ...\n");
}

/**
 * 初始化系统信号处理。
 * 注册 SIGINT/SIGQUIT/SIGTERM/SIGSEGV/SIGABRT/SIGBUS/SIGFPE/SIGILL/SIGTSTP
 * 将它们都指向统一的退出函数，确保程序在崩溃时能打印日志。
 */
void system_signal_init(){
    signal(SIGINT, signal_callback_func);
    signal(SIGQUIT, signal_callback_func);
    signal(SIGTERM, signal_callback_func);
    signal(SIGSEGV, signal_callback_func);
    signal(SIGABRT, signal_callback_func);
    signal(SIGBUS, signal_callback_func);
    signal(SIGFPE, signal_callback_func);
    signal(SIGILL, signal_callback_func);
    signal(SIGTSTP, signal_callback_func);
}

// typedef enum {
//     WIFI_DISCONNECTED=0,
//     WIFI_CONNECTED,
//     INVAILD_STATUS,
// }WIFI_STATUS_E;

// #define READ_WIFI_STATUS_CMD  "cat /sys/class/net/%s/carrier"
// #define WIFI_INTERFACE  "wlan0"

// WIFI_STATUS_E  get_wifi_status(void)
// {
//     int status = 0;
//     int bytes = 0;
//     char cmd[100];
//     char buf[500];
//     char *p =NULL;
//     memset(cmd,0,sizeof(cmd));
//     memset(buf,0,sizeof(buf));
//     sprintf(cmd,READ_WIFI_STATUS_CMD,WIFI_INTERFACE);
//     if(bytes > 0){
//         status = atoi(buf);
//         if(status == 1){
//             return WIFI_CONNECTED;
//         }
//         else if(status == 0){
//              return WIFI_DISCONNECTED;
//         }
//         else
//             return INVAILD_STATUS;
//     }
// }