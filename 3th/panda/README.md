# panda简介

panda是脚本语言解释器，它基于javascript，但裁剪了部分语法特性;

panda是轻量级语言，它被设计用来作为嵌入式硬件的解释器;

panda很容易学习。

## 动手操作

首先，需要一个解释器来执行你编写的脚本，目前有几种方式：

1. 在Mac或Pc机上运行panda的交互解释器（源代码编译生成）
2. 通过Mac或Pc连接到安装了cupkee OS的硬件板（这种方式下，你将能够直接操作硬件，更多内容请参考cupkee相关资料）

解释器在显示出一个漂亮（丑陋）的logo后，开始等待用户输入。
你可以键入panda/javascript语句，解释器将进行语法解析、编译为字节码、解释执行，最后将执行结果通过终端返回。

如果你使用panda交互解释器，看到的输出大概就是下面的样子

```
$ panda/repl
> 
```

## 编译panda解释器

### 获取源代码
```
cd your_workspace_path
git clone https://github.com/cupkee/panda.git
```

### 编译

在编译panda之前，你需要安装gcc编译器以及libreadline库

编译步骤：

1. 通过终端，进入panda源代码目录
2. 执行命令 make example
3. 正常情况下，在panda源代码目录中会出现一个build子目录，这里存放所有编译好的可执行文件
4. .../panda/build/example/repl 就是我们需要的交互解释器

## 把panda用到你自己的项目中

正向你所看到的，之前用到的交互解释器只是一个例子程序。

你完全可以将panda用到自己的项目中，作为配置解释器、命令行或其它。

## bug反馈
我非常需要实际使用中发现bug的反馈，以便不断对程序进行完善。

欢迎你通过以下两种方式提交bug：

1. 在github上提交issue（推荐）
2. 发EMail到bug-report@cupkee.cn

## 使用简介

panda一个裁剪板的javascript解释器，基础语法与javascript相同，非常易于学习掌握

### 支持的数据类型

* undefined类型
* NaN类型
* 布尔类型
* 数值类型
* 字符串类型
* 数组类型
* 对象类型

### 基本语句

#### var语句 － 定义变量

1. 定义变量a, b, c

```
var a, b, c
```

2. 定义变量并赋值

```
var a = 0, b, c = 'hello'
```

#### 表达式语句

1. panda解释器在计算表达式时与标准javascript区别

    不同类型的值进行运算时，不会进行类型转换，直接返回NaN（非数字）

2. 运算符

  * ++ -- () [] .
  * (单目) - ~ !
  * * / %
  * + -
  * & | ^ 
  * << >> 
  * < > >= <=
  * == !=
  * &&  || 
  * = += -= * = /= %= &= |= ^= <<= >>= 
  * ? :
  * , 

3. 基本表达式举例

```
> 1 + 1
2
> (10 + 5) * 3 / 5
9
> 1, 2, 3, 5 + 6
11
> 1 > 2
false
> 'hello' + 'world'
"helloworld"
> 'a1' < 'b2'
true
> 'hello' + 1
NaN
> 1 * 'hello'
NaN
...
```

#### if语句
if语句是根据条件选择不同分支处理的语句

```
if (true) {
    // always execute
} else {
    // never execute
}

if (a > 100) {
    ...
}
```

#### while语句
while语句是进行循环处理的语句

```
while(a++ < 100) {
    ...
}
```

#### break语句
break用于跳出循环

#### continue语句
continue用于跳转到循环条件判断

#### return语句
return 用于从函数跳出，并可指定返回值

### 函数

在panda中扩展了javascript定义函数的语法

#### 定义函数

1. 标准的定义方法

```
function fn() {
  // statements ...
}

// 匿名函数
var fn = function () {
  // statements ...
}
```

2. panda扩展语法

在panda中提供了def关键字，作为function的别名

```
def fn() {
  // statements ...
}

// 匿名函数
var fn = def () {
}
```

在panda中定义单语句函数时，可以不带"{}"

```
def fn() return 0;
var fn = def() return 1;
```

#### 调用函数

panda支持javascript程序中常用的即时调用

```
(def(x, y) {
    ...
    return x + y;
})(100, 50);            // return 150
```

#### 关于new关键字

在panda中不支持new的相关语义，但保留new关键字(不要把"new"作为变量或函数名)。

#### 闭包

panda支持闭包

```
var fn = (def(n) {
    var c = n;
    return def() return c++;
})(100);

fn() // 100
fn() // 101
```

### 复合数据类型

* panda中的数组和对象与javascript非常相似，但目前只提供了有限的方法。(一个好消息是：panda是开源代码，你可以随意添加需要的方法)

* Buffer对象来源于nodejs, panda同样也没有实现完整操作接口

* Date对象目前没有实现, 但已经在计划之中

* panda对复合类型的支持还在持续完善中, 参考test和example中的代码了解更多（不用担心，代码非常容易理解）

以下是一些使用复合对象的例子

```
/* array example */
var a = [];         // 定义空数组，赋值到变量a

a.push(1);
a.push('hello');
a.length();         // 2
a[0]                // 1
a[1]                // 'hello'
a[2]                // undefined
a.shift()           // 1
a.length()          // 1
a.pop()             // 'hello'
a.length()          // 0
a[0]                // undefined
a.unshift('hi');
a.push('panda');
a.length()          // 2
a[0]                // 'hi'
a[1]                // 'panda'

a.foreach(def(value, index) {
    ...
});

/* object example */
var o = {};         // 定义空对象，赋值到变量o

o.length();         // 0
o.a                 // undefined
o.a = 1;            // 1
o.a                 // 1
o['a']              // 1
o['a'] = 'hello     // 'hello
o['a']              // 'hello
o.length();         // 1
o.foreach(def(key, value) {
    ...
});

```

### 定义原生函数

请参考test和example中的代码，非常简单。


## 总结和提示

panda目前已经具备了解释器库的基本特性，而且很容易使用，可以用在很多项目中实现很有意思的功能

但以下几点需要注意：

1. panda还不完善，接口没有达到稳定状态，后续还将保持更新

2. panda首先被设计用在资源有限的嵌入式处理器，最小4k内存就可以工作，但对于大内存条件没有经过良好测试

3. panda内建了内存管理和垃圾收集机制，但当前的gc算法对大内存环境没有进行优化，随着内存使用量的增大gc占用的时间会有较大增长

4. panda不依赖任何操作系统层的支持, 整合到应用中非常简单，但操作系统提供的高级特性(如：多线程/任务，事件处理)需要集成开发人员处理

5. panda被cupkee(一个开源智能硬件操作系统)用来作为解释器，它的特性和开发进度是被cupkee驱动的

## 帮助panda成长

1. 使用并反馈bug或特性需求

2. 为panda提交代码

3. 其它你觉得有帮助的任何事


