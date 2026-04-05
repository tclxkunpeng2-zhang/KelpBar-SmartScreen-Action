
# README

## 环境要求

```
cmake >= 3.15
如果本地查询cmake --version小于改版本，可以按下方方法升级指定版本
$ wget http://www.cmake.org/files/v3.15/cmake-3.15.3.tar.gz
$ tar -xvzf cmake-3.15.3.tar.gz
$ cd cmake-3.15.3
$ ./configure
$ make
$ sudo make install
$ cmake --version

注意：使用linux仿真时，需要先安装环境依赖
sudo apt-get install build-essential libsdl2-dev -y

```

## 编译说明

```
1、指定交叉编译工具链工具链库位置
修改build.sh中toolchain_path的位置，改为你本机路径
toolchain_path="/home/xiaozhi/t113-v1.1/prebuilt/rootfsbuilt/arm/toolchain-sunxi-glibc-gcc-830/toolchain/bin"

2、编译相关
编译t113应用
./build.sh -t113
编译linux应用
./build.sh -linux
删除编译信息
./build.sh -clean

如果需要配置CMakeLists和屏幕分辨率相关参数，需要执行./build.sh -clean后再重新编译
如果需要切换板卡和PC机应用编译，需要执行./build.sh -clean后再重新编译

3、编译完成后，生成的应用在
build/app/demo

4、推到设备端运行即可
adb push platform/t113/lib/* /usr/lib/  #仅第一次需要push，不修改无需重新push
adb push build/app/res/* /usr/res/      #仅第一次需要push，不修改无需重新push
adb push build/app/demo /usr/bin/

 vi /etc/init.d/rc.final 
 ./usr/bin/demo & 
修改后，记得保存，最好用reboot重启确保可以完全写入



```

## 修改记录

学习进度
已完成 `page_about_us` 页面结构分析、`about` 相关图片尺寸确认，以及替换图片的压缩处理方案梳理。

修改内容
分析了 `page_about_us` 页面中的标题、二维码、背景图资源引用位置；确认 `icon_about_us.png` 为 `32x32`、`icon_qrcode.png` 为 `163x161`、`bg_about_us.png` 为 `280x280`；补充了大图替换时的压缩与缩放建议，并定位了 `t113` 交叉编译报错中 `-lz` 缺失的原因。

修改人
`zkp_vencent`
