/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:03 
 * @Last Modified by:   xiaozhi 
 * @Last Modified time: 2024-09-24 23:31:03 
 */

/* 页面公共工具函数实现
 * 提供页面切换时清理当前页面的通用接口。
 */
#include <stdio.h>
#include "page_conf.h"

/**
 * 删除当前活动屏幕上所有子对象并重置 LVGL 样式。
 * 用于页面跳转前清理旧页面的 UI 资源，避免内存泄漏。
 * @param style  需要被重置的 lv_style 对象指针
 */
void delete_current_page(lv_style_t *style){
    lv_obj_t * act_scr = lv_scr_act();          /* 获取当前活动屏幕 */
    lv_disp_t * d = lv_obj_get_disp(act_scr);
	if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
	{
		lv_obj_clean(act_scr);   /* 清除屏幕上所有子控件 */
        lv_style_reset(style);   /* 释放样式占用的内存 */
	}
}