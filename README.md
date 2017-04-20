# 介绍

cupkee是一个C语言编写，专门为嵌入式硬件设计的微型操作系统。

cupkee提供类似node的运行环境，使用内嵌的javascript解释器执行应用程序脚本。

cupkee在硬件上提供了REPL，开发者可以对硬件即时编程并获得响应。

cupkee小巧、简单

<!-- more -->

[关于cupkee的解释器](https://github.com/cupkee/panda)

## 支持的处理器

* stm32f103 (当前的BSP是按照stm32f103rc的资源定义实现的)


## 硬件编程

cupkee系统的编程非常简单。

首先，将装有cupkee系统的硬件通过USB连接到你的电脑。

然后, 开始编程

### 使用REPL

cupkee使用USB-CDC作为console，当前主流桌面操作系统都可免驱识别。

打开串口终端，键入Enter将看cupkee的logo信息和命令提示符。

输入js语句，cupkee会为你执行并返回结果

### 安装应用脚本

cupkee正常连接到电脑后，会出现卷标为cupdisk的外挂硬盘。

写好硬件应用脚本后，只需将其拖入此硬盘，即可完成安装。

### mac用户

mac是非常方便的开发平台，其上自带的screen应用即可作为与cupkee的通信终端。(这是我在开发cupkee时的用法)

完整的连接过程如下：

1. 用usb线连接cupkee板和mac
2. 打开mac上的终端程序
3. 在终端中输入命令screen /dev/cu.usbmodemCUPKE1 baudrate (波特率可以随便输入)
4. screen正常运行后，会清除终端的历史内容，为用户呈现一个干净的新终端界面
5. 键入Enter或其它任意，你将看到cupkee在终端中打印的logo，和输入提示符

### 非mac用户 或者不喜欢screen简陋功能

你可以使用其它常用的应用软件，如：putty，xshell，超级终端...

* ubuntu12.04以上版本：无需驱动

* windows10, windows8: 无需安装驱动，连接即可使用

较老电脑操作系统不能直接识别usb cdc设备，使用前需安装驱动程序。(不幸的是，目前没有驱动程序)

## cupkee使用简介

完成上述操作，就可以开始进行硬件编程了。

### 使用解释器

* 进行简单的计算

```
> 100 / 20 + 2
7
>
```

* 定义和使用变量

```
> var a = 1, b = 2;
undefined
> a
1
> a + b
3
> a = "hello"
"hello"
> b = "world"
"world"
> a + " " + b
"hello world"
```

* 定义和使用函数

```
> function fn(x, y) {
.   return x + y
. }
<function>
> fn(a, b)
3
```

### 操作硬件

* 控制指示灯

```
led(1)              // 设置led引脚为高电平
led(0)              // 设置led引脚为低电平
led()               // 反转led引脚电平
```

* 操作GPIO

```
pinMap(0, 0, 1)     // 将GPIO引脚PA1,映射为PIN0
pinMap(1, 0, 2)     // 将GPIO引脚PA2,映射为PIN1

var pin = Device('pin', 0) // 申请pin设备实例0

pin.config('num',   2)
pin.config('start', 1)     // 设置pin设备管理的引脚: PIN0, PIN1
pin.config('dir', 'in')    // 设置pin方向: 输入(in)，输出(out)，双向(dual)

pin.enable()

// 读取pin引脚值
pin.get()                 // PIN0 PIN1
pin.get(0)                // PIN1
pin.get(1)                // PIN2
pin[0]                    // PIN1
pin[1]                    // PIN2

pin.listen('data', function(state) {   // 注册引脚电平变化处理函数
  if (state[0]) led()
})
```

* 操作设备(pwm, uart, i2c, adc, timer, counter, pulse)

```
var pwm = Device('pwm', 0) // 申请pwm设备实例0
...
// 设置pwm周期为1000ms
pwm.config('period', 1000)
// 使能pwm
pwm.enable()
...
pwm.write(0, 10)           // 设置pwm通道0占空比为10:990
pwm.write(0, 1000)         // 设置pwm通道0占空比为1000:0


...
```

更多信息, 请参考后续介绍及cupkee函数及设备手册

### cupkee提供的原生函数

cupkee提供了一组原生函数供开发者使用

* print

print函数被设计为一个即时帮助工具，它可以用来打印变量的内容，和当前cupkee支持的原生函数

* systicks

cupkee内建有系统定时器，每秒1000次滴答（每毫秒一次）,作为系统程序的通用同步工具。

systicks函数返回系统启动后的总滴答数。

```
> while(1) {
    if (systicks() > 10000) {
        ...
        break;
    }
}
```

* ledMap & led

点亮指示灯是硬件调整最基本的手段，cupkee为此专门提供了ledMap和led两个原生函数进行支持。

*ledMap*  用于指定指示灯使用的GPIO引脚。

*led*     用于控制指示灯引脚电平：
    1. 不带参数调用时，反转引脚电平
    2. 传入真值（1, true, ...）时，引脚设为高电平
    3. 传入假值（0, false, ...）时，引脚设为低电平

* pinMap & pin设备

采用相同处理器的不同的硬件板卡的引脚使用方案，往往并不同。cupkee需要一种处理机制，让一个处理器的固件程序支持多种板卡。

原生函数pinMap就是为此而来，它可以将任意GPIO引脚映射到cupkee内建的抽象PIN[0-15]。PIN0-15］可以分配给pin设备使用。

* setTimeout, setInterval, clearTimeout, clearInterval

cupkee提供了一组定时器函数，它们相对于使用systicks管理程序同步更加有效

1. setTimeout

    注册延时(回调)函数，在指定时间后执行

2. setInveral

    注册周期(回调)函数，以指定的时间间隔周期执行

3. clearTimeout

    清除延时函数

4. clearInterval

    清除周期函数

``` 
// 定义函数f1，并在1000毫秒后执行
var t = setTimeout(def f1() {
    ...
}, 1000)
...
// 定义函数f2，每1000毫秒执行一次
var i = setInterval(def f2() {
    ...
}, 1000)
...
// 清除延时函数, 参数为setTimeout的返回值
clearTimeout(t)
...
// 清除周期函数, 参数为setInterval的返回值
clearInterval(t)
...
// 清除所有延时函数
clearTimeout()
...
// 清除所有周期函数
clearInterval()
```

### cupkee上的设备

原生函数Device用来创建和查看可用设备。

```
> Device()              // 打印设备列表
Device id conf inst
* pin   0    3    2
* adc   0    3    2
...
undefined
>
> var pwm, key, adc
> pwm = Device('pwm', 1) // 申请pwm设备实例1
> key = Device('pin', 1) // 申请pin设备实例1
> adc = Device('adc', 0) // 申请adc设备实例0
```

## 获取cupkee固件程序

1. 从cupkee网站下载

2. 通过源代码编译


## bug反馈

在使用中发现bug的反馈，可以帮助我对程序持续进行完善。

欢迎你通过以下两种方式提交bug：

1. 在github上提交issue（推荐）
2. 发EMail到bug-report@cupkee.cn

## 更多内容

[请参考cupkee网站的手册](http://www.cupkee.cn)

## 帮助cupkee成长

1. 使用cupkee并反馈bug或特性需求

2. 其它你觉得有帮助的任何事

