
# KelpBar-SmartScreen-Action

## 项目说明

这是一个基于 LVGL 的长条屏应用工程，支持 Linux 模拟运行和 T113 板端交叉编译。项目内包含主页面、系统设置、WiFi、场景联动、番茄钟、关于页等多个子页面。

## 环境要求

基础环境：

```bash
cmake >= 3.15
build-essential
```

Linux 模拟器依赖：

```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev -y
```

图片处理工具：

```bash
sudo apt-get install imagemagick pngquant -y
```

如果本地 `cmake --version` 低于要求版本，可自行升级。

## 编译说明

1. 修改 [build.sh](/home/zkp/KelpBar-SmartScreen-Action/build.sh) 中的 `toolchain_path`，替换为本机交叉编译工具链路径。

```bash
toolchain_path="/home/xiaozhi/t113-v1.1/prebuilt/rootfsbuilt/arm/toolchain-sunxi-glibc-gcc-830/toolchain/bin"
```

2. 常用编译命令：

```bash
./build.sh -linux
./build.sh -t113
./build.sh -clean
```

3. 编译产物路径：

```bash
build/app/demo
```

4. 推送到设备：

```bash
adb push platform/t113/lib/* /usr/lib/
adb push build/app/res/* /usr/res/
adb push build/app/demo /usr/bin/
```

5. 开机自启动：

```bash
vi /etc/init.d/rc.final
./usr/bin/demo &
```

## 当前修改

学习进度
已完成 `page_about_us` 页面结构重构、图片尺寸与压缩方案确认，以及 `t113` 编译环境错误排查。

修改内容
将 [page_about_us.c](/home/zkp/KelpBar-SmartScreen-Action/app/ui/page_about_us.c) 从二维码展示页改造成设备信息页，移除了居中的二维码和说明文字，新增 CPU 占有率、内存占有率、Flash 占有率、网络 IP 信息，并重排为适合长条屏的 `2x2` 信息卡片布局；同时确认并记录了 `icon_about_us.png` 为 `32x32`、`icon_qrcode.png` 为 `163x161`、`bg_about_us.png` 为 `280x280` 的资源尺寸；补充了使用 `convert` 与 `pngquant` 处理图片缩放和有损压缩的方法；排查 `./build.sh -t113` 报错后，确认当前问题来自交叉工具链缺少 `libz` 等依赖库，而非页面代码本身。

修改人
`zkp_vencent`

## 已知问题

当前 `t113` 交叉编译仍可能失败，典型报错为：

```text
ld: cannot find -lz
```

问题定位在 [platform/t113/t113.cmake](/home/zkp/KelpBar-SmartScreen-Action/platform/t113/t113.cmake) 中全局链接了 `-lz` 和 `-lbz2`，但当前工具链目录未提供对应 ARM 库或未正确配置 sysroot。

## 参考文档

学习整理文档见 [docs/kelpbar_learning_notes.md](/home/zkp/KelpBar-SmartScreen-Action/docs/kelpbar_learning_notes.md)。
