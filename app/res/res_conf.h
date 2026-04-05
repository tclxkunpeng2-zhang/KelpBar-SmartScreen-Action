/*
 * @Author: xiaozhi 
 * @Date: 2024-09-24 23:31:25 
 * @Last Modified by:   xiaozhi 
 * @Last Modified time: 2024-09-24 23:31:25 
 */

/* 资源路径配置头文件
 * 定义字体、图片、音乐资源的目录前缀宏。
 * PROJECT_RES_URL 由编译系统 CMakeLists.txt 注入，
 * T113 发布时对应 /usr/res/，Linux 模拟时对应 build/app/res/。
 */
#ifndef _RES_CONF_H_
#define _RES_CONF_H_

#define FONT_PATH  PROJECT_RES_URL "font/"   /* 字体文件目录 */
#define IMAGE_PATH PROJECT_RES_URL "image/"  /* 图片资源目录 */
#define MUSIC_PATH PROJECT_RES_URL "music/"  /* 音乐/音效目录 */

#endif