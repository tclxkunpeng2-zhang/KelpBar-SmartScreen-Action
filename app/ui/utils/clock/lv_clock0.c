
/* 指针时钟样式0实现
 * 使用表盘 + 时/分/秒针图片，通过 lv_img_set_angle 模拟转动。
 * 每秒通过 LVGL 定时器发送 LV_EVENT_REFRESH 事件触发更新。
 */

#include "lv_clock.h"

/* 时钟刺激回调：根据当前时间计算各针转动角度并更新图片旋转 */
static void refresh(lv_event_t* event){
    lv_clock_t *lv_clock  = (lv_clock_t *)lv_event_get_user_data(event);
    time(&lv_clock->timep);
    memcpy(&lv_clock->time_temp, localtime(&lv_clock->timep), sizeof(struct tm));
    // printf("h=%d m=%d s=%d\n",time_temp.tm_hour,time_temp.tm_min,time_temp.tm_sec);
    lv_img_set_angle(lv_clock->sec_img, lv_clock->time_temp.tm_sec * 60);    //3600/60 =60
    lv_img_set_angle(lv_clock->minute_img, lv_clock->time_temp.tm_min * 60); //3600/60 =60
    lv_img_set_angle(lv_clock->hour_img, lv_clock->time_temp.tm_hour * 300 + lv_clock->time_temp.tm_min*5); //3600/300 = 12
}

/* 定时器回调：每秒向表盘对象发送 REFRESH 事件 */
static void send_event(lv_timer_t * timer) { 
    lv_clock_t *lv_clock = (lv_clock_t *)timer->user_data;  // 从 timer 中获取用户数据
    if (lv_clock && lv_clock->dial_img) {
        lv_event_send(lv_clock->dial_img, LV_EVENT_REFRESH, NULL);
    }
}

/**
 * 初始化样式0 指针时钟组件。
 * 创建表盘图片为根，劁展时/分/秒针图片并居中对齐，
 * 最后启动 1s 定时器驱动刺激。
 */
lv_obj_t * init_clock0_obj(lv_obj_t* parent,lv_clock_t *lv_clock){
    lv_clock->dial_img = lv_img_create(parent);
    lv_img_set_src(lv_clock->dial_img, GET_IMAGE_PATH("icon_dial.png"));
    lv_obj_align(lv_clock->dial_img,  LV_ALIGN_LEFT_MID, 0, 5);

    lv_clock->hour_img = lv_img_create(lv_clock->dial_img);
    lv_img_set_src(lv_clock->hour_img, GET_IMAGE_PATH("icon_time_h.png"));
    lv_obj_align(lv_clock->hour_img,  LV_ALIGN_CENTER, 0, 0);

    lv_clock->minute_img = lv_img_create(lv_clock->dial_img);
    lv_img_set_src(lv_clock->minute_img, GET_IMAGE_PATH("icon_time_m.png"));
    lv_obj_align(lv_clock->minute_img,  LV_ALIGN_CENTER, 0, 0);

    lv_clock->sec_img = lv_img_create(lv_clock->dial_img);
    lv_img_set_src(lv_clock->sec_img, GET_IMAGE_PATH("icon_time_s.png"));
    lv_obj_align(lv_clock->sec_img,  LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(lv_clock->dial_img, refresh,LV_EVENT_REFRESH ,(void *)lv_clock);
    lv_event_send(lv_clock->dial_img,LV_EVENT_REFRESH,NULL);
    if(lv_clock->refresh_timer == NULL)
        lv_clock->refresh_timer = lv_timer_create((void*)send_event, 1000, (void *)lv_clock);
    return lv_clock->dial_img;
}

/* 销毁样式0 指针时钟，删除定时器防止内存泄漏 */
void deinit_clock0_obj(lv_clock_t *lv_clock){
    if(lv_clock->refresh_timer != NULL){
        lv_timer_del(lv_clock->refresh_timer);
        lv_clock->refresh_timer = NULL;
    }
}