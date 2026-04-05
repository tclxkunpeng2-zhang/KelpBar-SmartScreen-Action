/* 应用程序入口文件
 * 负责初始化所有子系统（显示、输入、字体、WiFi、BLE Mesh、HTTP、音频），
 * 并进入 LVGL 主循环。
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lvgl.h"
#include "font_conf.h"
#include "page_conf.h"
#include "device_data.h"
#include "ble_mesh.h"
#include "utils.h"
#include "http_manager.h"
#include "audio_player_async.h"

/* 声明平台相关的显示和输入驱动初始化函数 */
extern void lv_port_disp_init(bool is_disp_orientation);
extern void lv_port_indev_init(void);

/* LVGL 任务处理器返回的下次执行等待时间（毫秒） */
static uint32_t time_till_next = 1;

int main() {
    system_signal_init();       /* 初始化系统信号处理（捕获SIGINT、SIGSEGV等） */
    init_device_state();        /* 初始化设备全局状态并从文件读取持久化参数 */
    device_state_t *state = get_device_state();
    lv_init();                  /* 初始化 LVGL 图形库 */
    lv_port_disp_init(state->is_disp_orientation); /* 初始化显示驱动，支持横竖屏配置 */
    lv_port_indev_init();       /* 初始化触摸/输入设备驱动 */
    FONT_INIT();                /* 加载字体资源 */
    device_timer_init();        /* 初始化设备定时器（每秒触发一次，用于倒计时等） */
    init_async_audio_player();  /* 初始化异步音频播放器 */
#ifdef SIMULATOR_LINUX
    // Linux模拟器无ble mesh
    printf("Running in Linux simulator mode.\n");
#else
    printf("Running in T113 mode.111\n");
    wpa_manager_open();         /* 启动 wpa_supplicant WiFi 管理 */
    ble_mesh_init();            /* 初始化 BLE Mesh 串口通信 */
#endif
    http_request_create();      /* 创建 HTTP 网络请求线程 */
    init_page_main();           /* 跳转到主页面（表盘/菜单界面） */
    while (1) {
        time_till_next = lv_task_handler(); /* 驱动 LVGL 执行绘制和事件处理 */
        /**
         * 延时，保证cpu占有率不会过高
         */
        usleep(time_till_next*1000);
    }
    return 0;
}