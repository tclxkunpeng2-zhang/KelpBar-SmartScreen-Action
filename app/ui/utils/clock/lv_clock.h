/* 指针时钟组件头文件
 * 提供三种样式指针时钟的初始化/销毁接口。
 * 每种样式都使用 LVGL 图片转轴模拟时、分、秒针的转动。
 */
#ifndef _LV_CLOCK_H_
#define _LV_CLOCK_H_

#include <stdio.h>
#include "lvgl.h"
#include <time.h>
#include <sys/time.h>
#include "image_conf.h"

/* 时钟操作句柄：存储时间字段、表盘/时/分/秒针图片对象、定时器 */
typedef struct 
{
    time_t timep;               /* 时间戳 */
    struct tm time_temp;        /* 分解后的本地时间 */
    lv_obj_t * dial_img;        /* 表盘背景图片 */
    lv_obj_t * hour_img;        /* 时针图片 */
    lv_obj_t * minute_img;      /* 分针图片 */
    lv_obj_t * sec_img;         /* 秒针图片 */
    lv_timer_t * refresh_timer; /* 1秒刷新定时器 */
}lv_clock_t;

/* 初始化样式0/1/2 的指针时钟，返回表盘根对象指针 */
lv_obj_t * init_clock0_obj(lv_obj_t* parent,lv_clock_t *lv_clock);
lv_obj_t * init_clock1_obj(lv_obj_t* parent,lv_clock_t *lv_clock);
lv_obj_t * init_clock2_obj(lv_obj_t* parent,lv_clock_t *lv_clock);

/* 销毁对应样式时钟，停止定时器 */
void deinit_clock0_obj(lv_clock_t *lv_clock);
void deinit_clock1_obj(lv_clock_t *lv_clock);
void deinit_clock2_obj(lv_clock_t *lv_clock);

#endif
