/*
 * @Author: xiaozhi 
 * @Date: 2024-09-25 00:07:49 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 00:47:48
 */

/* 通用工具函数头文件
 * 提供时间格式化、系统信号处理等公共工具接口。
 */
#ifndef _UTILS_H_
#define _UTILS_H_

/**
 * 将分钟数转换为 "HH:MM" 格式字符串（字符正常显示）
 * @param time_min  输入分钟数
 * @return 指向静态内部缓冲区的字符串指针
 */
char *get_time_str(int time_min);

/**
 * 将分钟数转换为 "HH:MM" 格式字符串（冒号用黑色隐藏，实现闪烁效果）
 * @param time_min  输入分钟数
 * @return 指向静态内部缓冲区的字符串指针
 */
char *get_time_str_nosymbol(int time_min);

/**
 * 注册常见系统信号处理函数（SIGINT/SIGSEGV 等），确保程序异常时能正常退出。
 */
void system_signal_init(void);

#endif
