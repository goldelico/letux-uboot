1. 目录结构
./drivers/usb/gadget/hid/
├── f_hid.c
#设备function的具体实现，hid设备读/写接口。
├── f_hid.h
├── gpio.c
#按键的初始化和状态获取
├── hid.c
#hid设备驱动的实现。
├── Makefile
└── readme.txt

2. uboot usb hid 测试命令
2.1 键盘模式测试
2.1.1 开发板端进入uboot shell 模式执行一下命令（默认为键盘模式）：
# hid
2.1.2 打开类似记事本的程序，并将光标定位到记事本
2.1.3 按下hally2开发板的SW2按键，记事本有123输入。

2.2 鼠标模式测试
2.2.1 修改为鼠标模式
修改./common/cmd_hid.c中
const static enum hid_type hid_current_type = KEYBOARD;
为
const static enum hid_type hid_current_type = MOUSE;
2.2.2 按下hally2开发板的SW2按键，相当与鼠标左键按下。

3. 接口函数
3.1 hid设备的读写接口(f_hid.c)
/*
 * 功能：设备发往主机数据.
 * buff: 将要发送的数据.
 * len:  发送数据的长度
 * label: 设备类型(鼠标/键盘)
 * 返回值: -1 失败 0 成功
 */
int submit_status(const unsigned char *buff, const int len, const enum hid_type  label);

/*
 * 功能：读取主机数据.
 * label: 设备类型.
 * 返回值: 读取数据长度.
 */
ssize_t f_hidg_read(const enum hid_type label); 读取主机发往设备的数据

3.2 hid设备的注册与注销(hid.c)
/*
 * 功能：注册设备.
 * hid_des: hid设备描述符.
 * type: 设备类型(鼠标/键盘)
 * 返回值: -1 失败 0 成功
 */
int jz_usb_hid_register(const struct hidg_func_descriptor *hid_des, const enum hid_type type);

/*
 * 功能：注销.
 */
void jz_usb_hid_unregister(void);

3.3 gpio的初始化及状态获取(gpio.c)
/*
 * 功能：初始化GPIO
 * gpio_group_t: 要初始化的GPIO组.
 * offset: 第几个GPIO.
 * f: 模式
 * 返回值: -1 失败 0 成功
 */
int init_gpio(const enum gpio_group gpio_group_t, const int offset, const enum function f);

/*
 * 功能：获取GPIO状态
 * gpio_group_t: 获取GPIO状态的组.
 * offset: 第几个GPIO.
 * 返回值: 1 高电平 0 低电平
 */
int get_gpio_status(const enum gpio_group gpio_group_t, const int offset);

/*
 * 功能：获取按键是否按下
 * gpio_group_t: 获取GPIO状态的组.
 * offset: 第几个GPIO.
 * 返回值: 1 高电平 0 低电平
 */
int is_press_down(const enum gpio_group gpio_group_t, const int offset);

