# 简介

cupkee是一个C语言编写的微型操作系统，它专门设计用于微控制器硬件板。

cupkee建立了一个类似node的运行环境，在内部包含一个简化的javascript解释器作为shell。

cupkee为硬件板提供了即时交互的能力，开发者可以随时对硬件编程并获得即时响应。

cupkee将板卡上的硬件资源抽象为设备，并定义了一组标准方法供开发者使用。

[关于cupkee的解释器](https://github.com/cupkee/panda)

## 支持的硬件

cupkee对硬件进行了抽象，定义为BSP接口，理论上能够被移植到所有GCC编译器支持的处理器上。

支持的处理器：

* stm32f103 (当前的BSP是按照stm32f103rc的资源定义实现的)


## 连接cupkee板

因为大多数硬件板不具备人机交互设施，所以cupkee借用了板卡的usb作为console口；

使用PC或Mac通过usb连接硬件板，使用常规的终端程序即可与cupkee进行交互。

### 安装usb驱动

cupkee按照usb cdc规范，利用板卡上的usb口作为console。

一些较老电脑操作系统不能直接识别usb cdc设备，使用前需安装驱动程序。

#### GNU\linux用户

* ubuntu12.04以上版本：无需驱动

#### mac用户

* 无需驱动，即插即用

#### windwos用户

* windows10, windows8: 无需安装驱动，连接即可使用

* windows7, windowsxp: 市面上存在着各种usb转串口设备，理论上安装它们的驱动就可以 (因为没有windows环境，没有实际验证过)

### 连接cupkee

当你的电脑与cupkee硬件板通过usb线连接后（如有必要，安装驱动），你还需要一个应用程序作为与硬件通信的终端。

#### mac用户

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

#### 非mac用户 或者不喜欢screen简陋功能

你可以使用其它常用的应用软件，如：putty，xshell，超级终端...


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

	```
	// 更详细信息, 请参考后续介绍及cupkee函数及设备手册

    // 控制led
	> pinMap(0, 0, 0)   // 定义LED使用的引脚, 此处为PA0
	true                // 请根据你的板卡资源进行设置, pinMap(0, port, pin)
	> led(1)            // 设置led引脚为高电平
	undefined
	> led(0)            // 设置led引脚为低电平
	undefined
	> led()             // 反转led引脚电平
	undefined

    // 控制pwm
	> var pwm = device('pwm', 0) // 申请pwm设备实例0
	undefined
	> pwm.config('period', 1000) // 设置pwm周期为1000ms
	true
	> pwm.enable()               // 使能pwm
	true
    > pwm.write(0, 10)           // 设置pwm通道0占空比为10:990
	true
    > pwm.write(0, 1000)         // 设置pwm通道0占空比为1000:0
	true

    // 引脚管理
	> pinMap(1, 0, 1)            // 将GPIO PA1 映射为引脚1
	undefined
	> pinMap(2, 0, 2)            // 将GPIO PA2 映射为引脚2
	undefined
	> var key = device('pin', 0) // 申请pin设备实例0
	undefined
	// 为pin设备设置引脚, 使用pin1，pin2
	>key.config('pinStart', 1)
	true
	>key.config('pinNum', 2)
	true
	>key.config('dir', 'in')     // 设置pin方向: 输入(in)，输出(out)，双向(dual)
	true
	>key.enable()
	true
	>key[0]                      // 读取key引脚值
	0
	>key[1]                      // 读取key引脚值
	0
	>key('data', function(k) {   // 注册引脚电平变化处理函数
	.  if (k[0]) led()
	})
	true

	...
	```

### cupkee提供的原生函数

cupkee提供了一组原生函数供开发者使用

* show

show函数被设计为一个即时帮助工具，它可以用来打印变量的内容，和当前cupkee支持的原生函数

    1. 显示可用原生函数

    ```
    > show()
     * native_function_a
     * native_function_b
       ...
     * native_function_x
    undefined  // show函数的返回值, undefined表示该返回值无意义
    ```

    2. 显示变量内容

    ```
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

* setTimeout, setInterval, clearTimeout, clearInterval

cupkee提供了一组定时器函数，它们相对于使用systicks管理程序同步更加有效

1. setTimeout － 注册延时(回调)函数，在指定时间后执行

2. setInveral － 注册周期(回调)函数，以指定的时间间隔周期执行

3. clearTimeout － 清除延时函数

4. clearInterval － 清除周期函数

设置定时器

```
> setTimeout(def f1() {
    ...
}, 1000)                // 定义函数f1，并在1000毫秒后执行
1                       // setTimeout的返回值，定时器编号
> setInterval(def f2() {
    ...
}, 1000)                // 定义函数f2，每1000毫秒执行一次
2                       // setInverval的返回值，定时器编号
```

清除定时器

```
> clearTimeout(t)       // 清除延时函数, 参数为setTimeout的返回值
1                       // clearTimeout的返回值，表示清除了一个延时函数
> clearInterval(t)      // 清除周期函数, 参数为setInterval的返回值
1                       // clearInterval的返回值，表示清除了一个周期函数
> clearTimeout()        // 清除所有延时函数
n                       // 实际被清除的延时函数数量
> clearInterval()       // 清除所有周期函数
n                       // 实际被清除的周期函数数量
```

### cupkee上的设备

原生函数device用来创建和查看可用设备。

```
> device()              // 打印设备列表
device id conf inst
* pin   0    3    2
* adc   0    3    2
...
undefined
>
> var pwm, key, adc
> pwm = device('pwm', 1) // 申请pwm设备实例1
> key = device('pin', 1) // 申请pin设备实例1
> adc = device('adc', 0) // 申请adc设备实例0
```

#### pinMap & pin设备

采用相同处理器的不同的硬件板卡的引脚使用方案，往往并不同。cupkee需要一种处理机制，让一个处理器的固件程序支持多种板卡。

原生函数pinMap就是为此而来，它可以将任意GPIO引脚映射到cupkee内建的抽象PIN[0-15]。
其中PIN[0]固定用于led，PIN［1-15］可以分配给pin设备使用。


## 获取cupkee固件程序

1. 从cupkee网站下载

2. 通过源代码编译

## bug反馈

在使用中发现bug的反馈，可以帮助我对程序持续进行完善。

欢迎你通过以下两种方式提交bug：

1. 在github上提交issue（推荐）
2. 发EMail到bug-report@cupkee.cn

## 更多内容

[请参考cupkee网站的使用指导手册](http://www.cupkee.cn)

## 帮助cupkee成长

1. 使用并反馈bug或特性需求

2. 为cupkee贡献代码

3. 其它你觉得有帮助的任何事

