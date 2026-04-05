/* 下拉快捷栏头文件
 * 定义系统设置项目枚举和设置结构体，决层给 page_system_setting.c 和 page_pulldown_view.c 一同使用。
 */
#ifndef _PAGE_PULLDOWM_VIEW_H_
#define _PAGE_PULLDOWM_VIEW_H_

#include "lvgl/lvgl.h"

/* 系统设置项目类型枚举 */
typedef enum{
    SYSTEM_SETTING_BACKLIGHT = 0,   /* 屏幕亮度 */
    SYSTEM_SETTING_VOLUME,          /* 音量 */
}SYSTEM_SETTING_TYPE_E;

/* 系统设置项目信息结构体 */
typedef struct{
    SYSTEM_SETTING_TYPE_E type; /* 设置重类型 */
    char name[10];              /* 显示名称（如“亮度”/“音量”） */
    char *img_url;              /* 图标路径 */
    int value;                  /* 当前滑块值 */
    lv_obj_t *label;            /* 当前数值标签（如 "50%"） */
}system_setting_info_t;

/* 初始化下拉栏，view 为载体父对象 */
void init_page_pulldowm_view(lv_obj_t *view);
/* 销毁下拉栏 */
void deinit_page_pulldowm_view();
/* 获取下拉栏当前展开状态 */
bool get_is_pull_down();

#endif
