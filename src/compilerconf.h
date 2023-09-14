#pragma once

// 无警告区，用于包容稳定的第三方库头文件，如boost/ATL/WTL/STL/CxImage，不可自用
#define MSVC_NO_WARNING_AREA_BEGIN                      \
	__pragma(warning(push, 1))                        \
	__pragma(warning(disable: 4018 4100 4127 4191 4201 4204 4242 4244 4245 4290 4365))        \
	__pragma(warning(disable: 4505 4548 4555 4701 4702 4706 4819 4838 4917 4920 4995 4996))        \
	__pragma(warning(disable: 6001 6011 6031 6053 6202 6211 6246 6248 6255 6269 6282 6308 6323 6328 6334 6385 6386 6387 6400 6401)) \
    __pragma(warning(disable: 26454 28159))

#define MSVC_NO_WARNING_AREA_END                        \
	__pragma(warning(pop))

// 跳出警告堆栈的宏，在stdafx.h中用。
// 某些头文件(如boost/regex_token_iterator.hpp)可能对警告堆栈的使用很不小心，
// push多pop少，把我们锁在一个警告较少的栈帧中，这个宏就是为了跳出来
#define MSVC_CLEAR_WARNING_STACK         \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(disable: 4193)) __pragma(warning(pop))      \
	__pragma(warning(default: 4193))


// 在boost1.34中，包含boost/regex.hpp会造成pragma warning的栈不平，多了两次PUSH！
// 所以在cpp中如果包含boost/regex.hpp，请在包含之后写上这个宏，以免很多warning被boost给吃掉！
#define MSVC_WARNING_WORKAROUND__BOOST_1_34_REGEX __pragma(warning(pop)) __pragma(warning(pop))

// 打开所有VC缺省关闭的警告
// 工程中最好在工程选项中指定编译参数/Wall，以免遗漏某些警告
// 关闭不关心的警告
#define MSVC_TUNE_WARNINGS \
__pragma(warning(default: 4061 4062 4191 4242 4254 4263 4264 4265 4266 4287 4296)) \
__pragma(warning(default: 4302 4365 4514 4545 4546 4547 4548 4549 4555 4571)) \
__pragma(warning(default: 4623 4625 4626 4640 4668 4682 4710 4711 4820 4826 4836)) \
__pragma(warning(default: 4905 4906 4917 4928 4946)) \
\
__pragma(warning(disable: 4061 4127 4503 4514 4571 4623 4625 4626 4710 4820)) \
__pragma(warning(disable: 4505 4512))

// 4061) 在switch一个enum时没有在case中显式指定所有的enum中定义的数值
// 4127) 条件表达式是一个常数(在ATL_ENSURE,ATL_ENSURE_RETUEN_VAL中用到)
// 4355) this指针用在初始化列表里，项目里用得非常多
// 4503) 模板展开后的标识符过长
// 4514) 没有用到的内联函数被优化掉
// 4623 4625 4626) 因为父类构造函数/拷贝构造函数/赋值操作符不可见,所以无法为子类生成默认的
// 4820) 在类(结构)的相邻成员间插入了padding
// 4505) 未被引用到的函数被优化掉
// 4512) 缺省的赋值操作符不能被生成


#define MSVC_CONFIGURE_WARNINGS \
	MSVC_CLEAR_WARNING_STACK \
	MSVC_TUNE_WARNINGS

// 基础设施

#ifndef WIDESTRING
#define WIDESTRING2(str) L##str
#define WIDESTRING(str) WIDESTRING2(str)
#endif

#ifndef STRINGIZE
#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)
#endif

#ifndef COMPILE_MESSAGE
#define COMPILE_MESSAGE(msg) \
	__pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ") :" msg))
#endif
