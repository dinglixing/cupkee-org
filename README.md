

cupkee是一个C语言编写的微型操作系统，它专门设计用于微控制器硬件板。

cupkee建立了一个类似node的运行环境，在内部包含一个简化的javascript解释器作为shell。

cupkee为硬件板提供了即时交互的能力，开发者可以随时对硬件编程并获得即时响应。

cupkee将板卡上的硬件资源抽象为设备，并定义了一组标准方法供开发者使用。

<!-- more -->

[关于cupkee的解释器](https://github.com/cupkee/panda)

## 支持的硬件

cupkee对硬件进行了抽象，定义为BSP接口，理论上能够被移植到所有GCC编译器支持的处理器上。

支持的处理器：

* stm32f103 (当前的BSP是按照stm32f103rc的资源定义实现的)


## 连接cupkee

cupkee借用了板卡的usb作为console, 当你的PC或Mac通过usb与硬件板连接后(可能需要安装驱动)，通过常规的终端程序即可与cupkee进行交互。

### mac用户

mac是非常方便的开发平台，其上自带的screen应用即可作为与cupkee的通信终端。(这是我在开发cupkee时的用法)

完整的连接过程如下：

1. 用usb线连接cupkee板和mac
2. 打开mac上的终端程序
3. 在终端中输入命令screen /dev/cu.usbmodemCUPKE1 baudrate (波特率可以随便输入)
4. screen正常运行后，会清除终端的历史内容，为用户呈现一个干净的新终端界面
5. 键入Enter或其它任意，你将看到cupkee在终端中打印的logo，和输入提示符

```
$ screen /dev/cu.usbmodemCUPKE1 115200

// screen will be clean
// and show cupkee logo
   ______               __
  / ____/__  __ ____   / /__ ___   ___
 / /    / / / // __ \ / //_// _ \ / _ \
/ /___ / /_/ // /_/ // ,<  /  __//  __/
\____/ \__,_// .___//_/|_| \___/ \___/
            /_/
> 
```

### 非mac用户 或者不喜欢screen简陋功能

你可以使用其它常用的应用软件，如：putty，xshell，超级终端...

### 安装usb驱动

cupkee按照usb cdc规范，利用板卡上的usb口作为console。

一些较老电脑操作系统不能直接识别usb cdc设备，使用前需安装驱动程序。

* MacOS: 无需驱动，即插即用

* ubuntu12.04以上版本：无需驱动

* windows10, windows8: 无需安装驱动，连接即可使用

* windows7, windowsxp: 市面上存在着各种usb转串口设备，理论上安装它们的驱动就可以 (因为没有windows环境，没有实际验证过)


## cupkee使用简介

完成上述操作，就可以开始进行硬件编程了。

如:

* 进行简单的计算

	```
	> 100 / 20 + 2
	7
	>
	```

* 定义变量和函数

	```
	> var a = 1, b = 2;
	undefined
	> function fn(x, y) {
	.   return x + y
	. }
	<function>
	> fn(a, b)
	3
	```

* 使用原生函数控制硬件

	```javascript

    // LED指示灯
    // 定义LED使用的引脚, 此处为PA0
    // 请根据你的板卡资源进行设置, ledMap(port, pin)
	ledMap(0, 0)
    ...
	led(1)            // 设置led引脚为高电平
	led(0)            // 设置led引脚为低电平
	led()             // 反转led引脚电平

    // pwm
	var pwm = Device('pwm', 0) // 申请pwm设备实例0
    ...
    // 设置pwm周期为1000ms
	pwm.config('period', 1000)
    // 使能pwm
	pwm.enable()
    ...
    pwm.write(0, 10)           // 设置pwm通道0占空比为10:990
    pwm.write(0, 1000)         // 设置pwm通道0占空比为1000:0

    // 引脚管理
    // 将GPIO引脚PA1,映射为PIN1
    // 将GPIO引脚PA2,映射为PIN2
	pinMap(1, 0, 1)
	pinMap(2, 0, 2)
    ...

	var pin = Device('pin', 0) // 申请pin设备实例0

	// 设置pin设备管理的引脚: PIN1-2
    // 设置pin方向: 输入(in)，输出(out)，双向(dual)
	pin.config('num',   2)
	pin.config('start', 1)
	pin.config('dir', 'in')
    ...
	pin.enable()
    ...
    // 读取pin引脚值
    pin.read()                 // PIN1-2
    pin.read(0)                // PIN1
    pin.read(1)                // PIN2
	pin[0]                     // PIN1
	pin[1]                     // PIN2

	pin.listen('data', function(state) {   // 注册引脚电平变化处理函数
	  if (state[0]) led()
	})

	// 更多信息, 请参考后续介绍及cupkee函数及设备手册
	...
	```

### cupkee提供的原生函数

cupkee提供了一组原生函数供开发者使用

#### show

show函数被设计为一个即时帮助工具，它可以用来打印变量的内容，和当前cupkee支持的原生函数
    
```
    // 1. 显示可用原生函数
    > show()
     * native_function_a
     * native_function_b
       ...
     * native_function_x
    undefined  // show函数的返回值, undefined表示该返回值无意义
```

```
    // 2. 显示变量内容
    > show(1)
    1
    undefined  // show函数的返回值, undefined表示该返回值无意义
    > show('hello')
    "hello"
    undefined  // show函数的返回值, undefined表示该返回值无意义
    > show([1, 2, 3])
    [
      [0]:1
      [1]:2
      [2]:3
    ]
    undefined  // show函数的返回值, undefined表示该返回值无意义
    > var a = 0;
    undefined  // var语句的返回值, undefined表示该返回值无意义
    > show(a)
    0
    undefined  // show函数的返回值, undefined表示该返回值无意义
```

#### systicks

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

#### ledMap & led

点亮指示灯是硬件调整最基本的手段，cupkee为此专门提供了ledMap和led两个原生函数进行支持。

* ledMap

  用于指定指示灯使用的GPIO引脚。

* led

    用于控制指示灯引脚电平：

    1. 不带参数调用时，反转引脚电平
    2. 传入真值（1, true, ...）时，引脚设为高电平
    3. 传入假值（0, false, ...）时，引脚设为低电平


#### setTimeout, setInterval, clearTimeout, clearInterval

cupkee提供了一组定时器函数，它们相对于使用systicks管理程序同步更加有效

1. setTimeout

    注册延时(回调)函数，在指定时间后执行

2. setInveral

    注册周期(回调)函数，以指定的时间间隔周期执行

3. clearTimeout

    清除延时函数

4. clearInterval

    清除周期函数

``` javascript
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

#### pinMap & pin设备

采用相同处理器的不同的硬件板卡的引脚使用方案，往往并不同。cupkee需要一种处理机制，让一个处理器的固件程序支持多种板卡。

原生函数pinMap就是为此而来，它可以将任意GPIO引脚映射到cupkee内建的抽象PIN[0-15]。
PIN［1-15］可以分配给pin设备使用。


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

1. 使用并反馈bug或特性需求

2. 为panda提交代码

3. 其它你觉得有帮助的任何事

