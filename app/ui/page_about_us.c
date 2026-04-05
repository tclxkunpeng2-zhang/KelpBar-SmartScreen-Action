/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:09 
 * @Last Modified by: xiaozhi
 * @Last Modified time: 2024-09-25 22:46:54
 */

/* 关于页面实现
 * 展示设备运行信息，包括 CPU、内存、Flash 和网络 IP。
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "lvgl.h"
#include "color_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"

static lv_style_t com_style;
static lv_obj_t *cpu_value_label = NULL;
static lv_obj_t *mem_value_label = NULL;
static lv_obj_t *flash_value_label = NULL;
static lv_obj_t *ip_value_label = NULL;
static lv_timer_t *info_timer = NULL;
static uint64_t prev_total_ticks = 0;
static uint64_t prev_idle_ticks = 0;

typedef struct {
    lv_obj_t *row;
    lv_obj_t *title;
    lv_obj_t *value;
} info_item_t;


static void com_style_init(){
    lv_style_init(&com_style);
    if(lv_style_is_empty(&com_style) == false)
        lv_style_reset(&com_style);
    lv_style_set_bg_color(&com_style,APP_COLOR_BLACK);
    lv_style_set_radius(&com_style,0);
    lv_style_set_border_width(&com_style,0);
    lv_style_set_pad_all(&com_style,0);
}

static void obj_font_set(lv_obj_t *obj,int type, uint16_t weight){
    lv_font_t* font = get_font(type, weight);
	if(font != NULL)
		lv_obj_set_style_text_font(obj, font, 0);
}

static void deinit_info_timer(void){
    if(info_timer != NULL){
        lv_timer_del(info_timer);
        info_timer = NULL;
    }
}

static void back_btn_click_event_cb(lv_event_t * e){
    deinit_info_timer();
    delete_current_page(&com_style);
    init_page_main();
}

static void page_delete_event_cb(lv_event_t * e){
    LV_UNUSED(e);
    deinit_info_timer();
}

static lv_obj_t * init_title_view(lv_obj_t *parent){
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_set_align(cont,LV_ALIGN_TOP_MID);
    lv_obj_add_flag(cont,LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *back_img = lv_img_create(cont);
    lv_img_set_src(back_img,GET_IMAGE_PATH("icon_back.png"));
    lv_obj_set_align(back_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_left(back_img,20,LV_PART_MAIN);
    lv_obj_set_style_pad_top(back_img,20,LV_PART_MAIN);

    lv_obj_t *menu_img = lv_img_create(cont);
    lv_img_set_src(menu_img,GET_IMAGE_PATH("icon_about_us.png"));
    lv_obj_set_align(menu_img,LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_pad_top(menu_img,20,LV_PART_MAIN);
    lv_obj_align_to(menu_img,back_img,LV_ALIGN_OUT_RIGHT_MID,20,0);

    lv_obj_t *title = lv_label_create(cont);
    obj_font_set(title,FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(title,"zkp 复刻 KelpBar 智能屏");
    lv_obj_set_style_text_color(title,APP_COLOR_WHITE,0);
    lv_obj_align_to(title,menu_img,LV_ALIGN_OUT_RIGHT_MID,20,3);

    lv_obj_add_event_cb(cont,back_btn_click_event_cb,LV_EVENT_CLICKED,NULL);

    return cont;
}

static bool get_cpu_usage(float *usage){
    FILE *fp = fopen("/proc/stat", "r");
    if(fp == NULL){
        return false;
    }

    char line[256] = {0};
    if(fgets(line, sizeof(line), fp) == NULL){
        fclose(fp);
        return false;
    }
    fclose(fp);

    uint64_t user = 0;
    uint64_t nice = 0;
    uint64_t system = 0;
    uint64_t idle = 0;
    uint64_t iowait = 0;
    uint64_t irq = 0;
    uint64_t softirq = 0;
    uint64_t steal = 0;
    if(sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
              &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 4){
        return false;
    }

    uint64_t idle_ticks = idle + iowait;
    uint64_t total_ticks = user + nice + system + idle + iowait + irq + softirq + steal;
    if(prev_total_ticks == 0 || total_ticks <= prev_total_ticks){
        prev_total_ticks = total_ticks;
        prev_idle_ticks = idle_ticks;
        *usage = 0.0f;
        return true;
    }

    uint64_t total_diff = total_ticks - prev_total_ticks;
    uint64_t idle_diff = idle_ticks - prev_idle_ticks;
    if(total_diff == 0){
        *usage = 0.0f;
    }else{
        *usage = (float)(total_diff - idle_diff) * 100.0f / (float)total_diff;
    }

    prev_total_ticks = total_ticks;
    prev_idle_ticks = idle_ticks;
    return true;
}

static bool get_mem_usage(float *usage){
    FILE *fp = fopen("/proc/meminfo", "r");
    if(fp == NULL){
        return false;
    }

    char key[64] = {0};
    char unit[32] = {0};
    unsigned long value = 0;
    unsigned long total_kb = 0;
    unsigned long available_kb = 0;

    while(fscanf(fp, "%63s %lu %31s", key, &value, unit) == 3){
        if(strcmp(key, "MemTotal:") == 0){
            total_kb = value;
        }else if(strcmp(key, "MemAvailable:") == 0){
            available_kb = value;
        }

        if(total_kb > 0 && available_kb > 0){
            break;
        }
    }
    fclose(fp);

    if(total_kb == 0){
        return false;
    }

    if(available_kb > total_kb){
        available_kb = 0;
    }
    *usage = (float)(total_kb - available_kb) * 100.0f / (float)total_kb;
    return true;
}

static bool get_flash_usage(float *usage){
    struct statvfs fs_stat;
    if(statvfs("/", &fs_stat) != 0 || fs_stat.f_blocks == 0){
        return false;
    }

    unsigned long long total_blocks = fs_stat.f_blocks;
    unsigned long long free_blocks = fs_stat.f_bavail;
    if(free_blocks > total_blocks){
        free_blocks = 0;
    }

    *usage = (float)(total_blocks - free_blocks) * 100.0f / (float)total_blocks;
    return true;
}

static bool get_ip_address(char *ip_buf, size_t ip_buf_len){
    struct ifaddrs *ifaddr = NULL;
    if(getifaddrs(&ifaddr) != 0){
        return false;
    }

    bool found = false;
    struct ifaddrs *ifa = ifaddr;
    while(ifa != NULL){
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET){
            ifa = ifa->ifa_next;
            continue;
        }
        if(ifa->ifa_name == NULL || strcmp(ifa->ifa_name, "lo") == 0){
            ifa = ifa->ifa_next;
            continue;
        }

        struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
        if(inet_ntop(AF_INET, &addr->sin_addr, ip_buf, ip_buf_len) != NULL){
            found = true;
            break;
        }
        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifaddr);
    return found;
}

static void refresh_device_info(lv_timer_t * timer){
    LV_UNUSED(timer);

    char text_buf[64] = {0};
    float usage = 0.0f;

    if(cpu_value_label != NULL){
        if(get_cpu_usage(&usage)){
            snprintf(text_buf, sizeof(text_buf), "%.1f%%", usage);
        }else{
            snprintf(text_buf, sizeof(text_buf), "--");
        }
        lv_label_set_text(cpu_value_label, text_buf);
    }

    if(mem_value_label != NULL){
        if(get_mem_usage(&usage)){
            snprintf(text_buf, sizeof(text_buf), "%.1f%%", usage);
        }else{
            snprintf(text_buf, sizeof(text_buf), "--");
        }
        lv_label_set_text(mem_value_label, text_buf);
    }

    if(flash_value_label != NULL){
        if(get_flash_usage(&usage)){
            snprintf(text_buf, sizeof(text_buf), "%.1f%%", usage);
        }else{
            snprintf(text_buf, sizeof(text_buf), "--");
        }
        lv_label_set_text(flash_value_label, text_buf);
    }

    if(ip_value_label != NULL){
        if(get_ip_address(text_buf, sizeof(text_buf))){
            lv_label_set_text(ip_value_label, text_buf);
        }else{
            lv_label_set_text(ip_value_label, "未连接");
        }
    }
}

static void init_info_item(lv_obj_t *parent, info_item_t *item, const char *title_text, lv_obj_t **value_label){
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_size(card, 340, 52);
    lv_obj_add_style(card, &com_style, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x323232), 0);
    lv_obj_set_style_bg_opa(card, 64, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_pad_left(card, 14, 0);
    lv_obj_set_style_pad_right(card, 14, 0);
    lv_obj_set_style_pad_top(card, 10, 0);
    lv_obj_set_style_pad_bottom(card, 10, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    item->row = lv_obj_create(card);
    lv_obj_set_size(item->row, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(item->row, &com_style, 0);
    lv_obj_clear_flag(item->row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(item->row, LV_OPA_0, 0);
    lv_obj_set_flex_flow(item->row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item->row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    item->title = lv_label_create(item->row);
    obj_font_set(item->title, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(item->title, title_text);
    lv_obj_set_style_text_color(item->title, lv_color_hex(0xB8B8B8), 0);

    item->value = lv_label_create(item->row);
    obj_font_set(item->value, FONT_TYPE_LETTER, FONT_SIZE_TEXT_2);
    lv_label_set_text(item->value, "--");
    lv_obj_set_style_text_color(item->value, APP_COLOR_WHITE, 0);
    lv_obj_set_style_text_align(item->value, LV_TEXT_ALIGN_RIGHT, 0);

    *value_label = item->value;
}

static lv_obj_t * init_info_panel(lv_obj_t *parent){
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 740, 150);
    lv_obj_add_style(panel, &com_style, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_left(panel, 0, 0);
    lv_obj_set_style_pad_right(panel, 0, 0);
    lv_obj_set_style_pad_top(panel, 0, 0);
    lv_obj_set_style_pad_bottom(panel, 0, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(panel, 10, 0);
    lv_obj_set_style_pad_column(panel, 12, 0);

    info_item_t cpu_item = {0};
    info_item_t mem_item = {0};
    info_item_t flash_item = {0};
    info_item_t ip_item = {0};
    init_info_item(panel, &cpu_item, "CPU 占有率", &cpu_value_label);
    init_info_item(panel, &mem_item, "内存占有率", &mem_value_label);
    init_info_item(panel, &flash_item, "Flash 占有率", &flash_value_label);
    init_info_item(panel, &ip_item, "网络 IP", &ip_value_label);

    return panel;
}

void init_page_about_us()
{
    com_style_init();
    prev_total_ticks = 0;
    prev_idle_ticks = 0;
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &com_style, 0);
    lv_obj_clear_flag(cont,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(cont, page_delete_event_cb, LV_EVENT_DELETE, NULL);

    init_title_view(cont);

    lv_obj_t *bg_img = lv_img_create(cont);
    lv_img_set_src(bg_img,GET_IMAGE_PATH("bg_about_us.png"));
    lv_obj_align(bg_img,LV_ALIGN_RIGHT_MID,-2,0);

    lv_obj_t * section_title = lv_label_create(cont);
    obj_font_set(section_title, FONT_TYPE_LETTER, FONT_SIZE_TEXT_1);
    lv_label_set_text(section_title, "设备运行信息");
    lv_obj_set_style_text_color(section_title, APP_COLOR_WHITE, 0);
    lv_obj_align(section_title, LV_ALIGN_TOP_LEFT, 80, 74);

    lv_obj_t * info_panel = init_info_panel(cont);
    lv_obj_align(info_panel, LV_ALIGN_TOP_LEFT, 80, 110);

    refresh_device_info(NULL);
    info_timer = lv_timer_create(refresh_device_info, 2000, NULL);
}
