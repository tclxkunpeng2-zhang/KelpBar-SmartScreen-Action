# KelpBar 长条屏项目学习记录

## 一、项目概览

这个项目是一个基于 LVGL 的长条屏 UI 工程，支持两种运行方式：

- Linux 模拟器运行，便于本地调试 UI
- T113 板端交叉编译运行，面向真实设备部署

项目业务代码主要集中在 `app/ui` 目录下，每个 `page_xxx.c` 对应一个页面，页面切换入口统一在 `page_conf.h` 中声明，主菜单入口由 `page_main.c` 管理。

## 二、开发环境搭建

### 1. 基础依赖

在 Ubuntu / WSL 环境下，建议先安装以下工具：

```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev cmake imagemagick pngquant -y
```

其中：

- `build-essential` 用于编译
- `libsdl2-dev` 用于 Linux 模拟器运行
- `imagemagick` 用于缩放图片
- `pngquant` 用于 PNG 有损压缩

### 2. 交叉编译工具链

项目通过 [build.sh](/home/zkp/KelpBar-SmartScreen-Action/build.sh) 指定交叉编译器路径：

```bash
toolchain_path="/home/zkp/toolchain-sunxi-glibc-gcc-830/toolchain/bin"
```

如果路径不对，先改成自己本机的实际路径。

### 3. 编译命令

Linux 模拟器：

```bash
./build.sh -linux
```

T113 交叉编译：

```bash
./build.sh -t113
```

清理构建目录：

```bash
./build.sh -clean
```

## 三、代码结构梳理

### 1. 页面入口

页面初始化函数统一在 [page_conf.h](/home/zkp/KelpBar-SmartScreen-Action/app/ui/page_conf.h) 中声明，例如：

- `init_page_main()`
- `init_page_about_us()`
- `init_page_system_setting()`

### 2. 主页面与菜单跳转

[page_main.c](/home/zkp/KelpBar-SmartScreen-Action/app/ui/page_main.c) 负责：

- 构建长条屏主页面
- 显示时间、天气、状态图标
- 构建底部横向菜单
- 在点击菜单时跳转到不同子页面

### 3. 当前重点页面

本次改造的目标页是 [page_about_us.c](/home/zkp/KelpBar-SmartScreen-Action/app/ui/page_about_us.c)。

原页面功能：

- 显示二维码
- 显示课程说明文字
- 显示关于页背景图

改造后的功能：

- 移除二维码和课程引导文案
- 改为展示设备运行信息
- 展示项目更有价值的实时状态

## 四、页面改造记录

### 1. 原始资源分析

关于页主要图片资源尺寸如下：

- `icon_about_us.png`：`32x32`
- `icon_qrcode.png`：`163x161`
- `bg_about_us.png`：`280x280`
- `icon_menu_about.png`：`130x130`

这些尺寸对后续替换图片很重要，因为 LVGL 页面里的对齐和显示区域已经围绕这些尺寸写死或默认设计完成。

### 2. 图片替换与压缩

如果替换成 AI 生成的大图，常见问题是：

- 分辨率远大于页面实际显示尺寸
- PNG 文件体积达到几 MB
- 虽然显示时会缩小，但解码负担明显增加

例如：

- 原二维码图约 `163x161`
- 替换图可能是 `2080x2048`
- 页面实际显示仍只有很小一块区域

正确处理方式是先缩放，再压缩。

将图片裁成 `280x280`：

```bash
convert input.png -resize 280x280^ -gravity center -extent 280x280 temp_280.png
pngquant --quality=70-85 --output output_280.png --force temp_280.png
```

将图片裁成 `161x161`：

```bash
convert input.png -resize 161x161^ -gravity center -extent 161x161 temp_161.png
pngquant --quality=70-85 --output output_161.png --force temp_161.png
```

说明：

- `convert` 来自 ImageMagick 6
- `pngquant` 用于 PNG 有损压缩
- `70-85` 是较稳妥的压缩质量区间

## 五、关于页功能改造

### 1. 功能目标

为了让页面更有实用价值，将原来的“展示二维码”改为“显示设备运行信息”。

本次新增展示内容包括：

- CPU 占有率
- 内存占有率
- Flash 占有率
- 网络 IP 信息

### 2. 代码修改入口

这次改造主要集中在 [page_about_us.c](/home/zkp/KelpBar-SmartScreen-Action/app/ui/page_about_us.c)。

原始页面的核心结构比较简单：

- `init_title_view()` 负责顶部返回区域
- `init_page_about_us()` 负责创建二维码、说明文字和背景图

原页面中间部分主要是这两块：

- `lv_img_create(cont)` 创建二维码图片
- `lv_label_create(cont)` 创建“扫码了解更多课程”文字

本次改造时，首先删除了这两块居中内容，只保留：

- 顶部标题栏
- 右侧背景图

然后再补入新的设备状态展示区域。

### 3. 新增头文件与全局变量

为了获取系统运行信息，需要在页面文件中新增一些 Linux 头文件：

```c
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
```

这些头文件分别用于：

- 字符串处理
- 布尔值和整数类型
- 获取文件系统容量信息
- 获取网络接口地址

同时新增了几个页面级静态变量，用于保存 UI 标签对象和定时器：

```c
static lv_obj_t *cpu_value_label = NULL;
static lv_obj_t *mem_value_label = NULL;
static lv_obj_t *flash_value_label = NULL;
static lv_obj_t *ip_value_label = NULL;
static lv_timer_t *info_timer = NULL;
```

这些变量的作用是：

- 页面创建时记录数值标签对象
- 定时刷新时直接更新标签文本
- 页面退出时删除定时器，避免资源残留

### 4. 新增系统信息采集函数

为了让页面显示实时数据，本次增加了几个独立函数。

CPU 占有率：

```c
static bool get_cpu_usage(float *usage)
```

实现思路：

- 读取 `/proc/stat` 第一行 `cpu ...`
- 解析 user、system、idle 等字段
- 通过前后两次采样做差值计算 CPU 使用率

内存占有率：

```c
static bool get_mem_usage(float *usage)
```

实现思路：

- 读取 `/proc/meminfo`
- 提取 `MemTotal` 和 `MemAvailable`
- 计算 `(MemTotal - MemAvailable) / MemTotal`

Flash 占有率：

```c
static bool get_flash_usage(float *usage)
```

实现思路：

- 调用 `statvfs("/")`
- 获取文件系统总块数和可用块数
- 计算占用百分比

IP 地址：

```c
static bool get_ip_address(char *ip_buf, size_t ip_buf_len)
```

实现思路：

- 调用 `getifaddrs()`
- 遍历所有网络接口
- 过滤掉 `lo`
- 取第一个可用 IPv4 地址

### 5. 新增统一刷新函数

为了让页面自动更新，新增了：

```c
static void refresh_device_info(lv_timer_t * timer)
```

这个函数会：

- 调用前面的 4 个数据采集函数
- 将采集到的结果格式化成文本
- 更新到页面标签上

页面初始化时会先主动调用一次：

```c
refresh_device_info(NULL);
```

这样页面刚进入时就能看到数据，而不是等到第一次定时器触发。

### 6. 新增定时器与退出清理

页面中增加了定时器：

```c
info_timer = lv_timer_create(refresh_device_info, 2000, NULL);
```

含义是每 2 秒刷新一次设备状态。

为了避免页面退出后定时器还在跑，又增加了：

```c
static void deinit_info_timer(void)
```

并在两个地方做清理：

- 点击返回按钮时
- 页面对象删除时

这样可以避免：

- 切页后定时器悬挂
- 定时器继续操作已销毁的 LVGL 对象

### 7. UI 布局改造过程

本次 UI 改造不是一次到位的，而是经历了三轮调整。

第一轮：

- 用纵向列表展示 4 条信息
- 优点是实现快
- 问题是长条屏空白很多，而且只显示了两行

第二轮：

- 压缩字体、行高和面板间距
- 尝试让 4 行都塞进一个竖向面板
- 虽然能解决显示数量问题，但整体仍不够适合长条屏

第三轮：

- 改成 `2x2` 卡片布局
- 左侧新增“设备运行信息”标题
- 右侧保留背景图
- 卡片内部改成单行布局，标题在左、数值在右

最终效果更符合长条屏宽屏比例，也减少了页面大面积空白。

### 8. 关键修改点总结

如果从代码角度概括，本次页面改造的核心改动有 4 类：

1. 删除原二维码与文案展示逻辑
2. 增加 Linux 系统信息读取逻辑
3. 增加定时刷新与页面退出清理逻辑
4. 将中间区域重构为适合长条屏的状态卡片布局

### 9. 数据获取方式

页面中新增的数据采集逻辑主要依赖 Linux 系统接口：

- CPU：读取 `/proc/stat`
- 内存：读取 `/proc/meminfo`
- Flash：调用 `statvfs("/")`
- IP：调用 `getifaddrs()` 遍历网络接口

### 10. 界面布局调整

一开始采用纵向列表展示，但在长条屏场景下会造成大量空白，且行数较多时显示不完整。

最终调整为：

- 左侧标题区域显示“设备运行信息”
- 中间区域使用 `2x2` 卡片布局
- 右侧保留原背景图

这样能够更好利用横向空间，也更符合长条屏的观感。

### 11. 布局问题与修正

在改造过程中遇到两个典型问题：

第一，四项信息最初只显示两行。  
处理方式是压缩卡片高度、间距和字体尺寸，保证四项内容能落在单屏范围内。

第二，标题和数值发生重叠。  
处理方式是将卡片内部改为单行布局，标题在左，数值在右，而不是上下两行。

## 六、编译问题排查与解决

### 1. 报错现象

执行：

```bash
./build.sh -t113
```

报错类似：

```text
ld: cannot find -lz
```

### 2. 原因分析

这个错误不是业务代码导致的，而是交叉编译环境问题。

[platform/t113/t113.cmake](/home/zkp/KelpBar-SmartScreen-Action/platform/t113/t113.cmake) 中全局加入了：

```cmake
-lpthread -lfreetype -lrt -ldl -znow -zrelro -luapi -lm -lz -lbz2
```

但是当前工具链目录中没有找到对应 ARM 版：

- `libz`
- `libbz2`

因此 CMake 在编译器自检阶段就失败了。

### 3. 如何确认问题

可以用下面几条命令做确认。

确认工具链是否真的带了 `libz`：

```bash
find /home/zkp/toolchain-sunxi-glibc-gcc-830 -name 'libz.so*' -o -name 'libz.a'
```

确认工具链是否带了 `libbz2`：

```bash
find /home/zkp/toolchain-sunxi-glibc-gcc-830 -name 'libbz2.so*' -o -name 'libbz2.a'
```

查看编译器默认库搜索路径：

```bash
/home/zkp/toolchain-sunxi-glibc-gcc-830/toolchain/bin/arm-openwrt-linux-gcc -print-search-dirs
```

查看 sysroot：

```bash
/home/zkp/toolchain-sunxi-glibc-gcc-830/toolchain/bin/arm-openwrt-linux-gcc -print-sysroot
```

如果 `-print-sysroot` 为空，且 `find` 也找不到 `libz`，基本就可以判断是工具链或 sysroot 不完整。

### 4. 解决方案

这个问题的解决，不是修改业务页面代码，而是修正交叉编译环境。常见做法有三种。

方案一：换成完整 SDK 自带工具链  
这是最稳妥的方式。重新使用官方或配套 SDK 中完整的交叉编译环境，确保工具链里包含：

- `libz`
- `libbz2`
- `libfreetype`
- 对应头文件
- 正确的 sysroot

方案二：补齐 sysroot 并让 CMake 显式使用  
如果你已经有一套完整 rootfs 或 staging 目录，可以在 [platform/t113/t113.cmake](/home/zkp/KelpBar-SmartScreen-Action/platform/t113/t113.cmake) 里增加 `--sysroot` 或 `CMAKE_SYSROOT` 配置，让交叉编译器从正确的位置找库。

例如思路上可以改成：

```cmake
set(CMAKE_SYSROOT /path/to/your/sysroot)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --sysroot=${CMAKE_SYSROOT}")
```

同时保证该 sysroot 下存在：

```text
usr/lib/libz.so
usr/lib/libbz2.so
usr/lib/libfreetype.so
```

方案三：如果项目暂时不需要某些库，可临时删掉对应链接项  
这是临时方案，只适合验证构建链路是否畅通，不适合作为最终交付方案。

例如在 [platform/t113/t113.cmake](/home/zkp/KelpBar-SmartScreen-Action/platform/t113/t113.cmake) 中临时移除：

```cmake
-lz
-lbz2
```

但前提是：

- 项目运行时确实不会依赖这些库
- `freetype` 或其他组件不会间接依赖它们

否则即使通过了编译器自检，后续链接业务目标时仍然会失败。

### 5. 推荐解决路径

从实际工程经验看，推荐按下面顺序处理：

1. 先确认工具链目录里是否存在 `libz` / `libbz2`
2. 如果没有，优先换完整 SDK 或完整 rootfs
3. 如果有，但编译器找不到，就补 `sysroot`
4. 最后才考虑临时删掉 `-lz` / `-lbz2` 做验证

### 6. 当前结论

结论是：

- Linux 模拟器侧可以继续调试 UI
- T113 交叉编译需要补齐工具链库或正确配置 sysroot

也就是说，这次关于页代码修改本身不是导致编译失败的原因。

## 七、学习收获

这次改造比较典型，适合作为嵌入式 UI 工程入门案例，主要收获有：

- 学会快速梳理 LVGL 多页面工程结构
- 学会根据页面实际显示尺寸处理素材，而不是直接替换大图
- 学会通过 `/proc` 和系统接口读取运行信息并显示到 UI
- 学会区分“代码问题”和“交叉编译环境问题”
- 学会针对长条屏重新设计布局，而不是简单平移 PC 或手机页面思路

## 八、后续优化建议

后面如果继续完善这个页面，可以考虑加入：

- CPU 温度
- 系统运行时长
- WiFi 名称与信号强度
- 存储总量 / 已用量 / 剩余量
- 卡片图标和更明显的数据视觉层级

这会让页面更像一个真正的设备状态面板，而不仅仅是简单的信息列表。
