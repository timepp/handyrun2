#pragma once

// �޾����������ڰ����ȶ��ĵ�������ͷ�ļ�����boost/ATL/WTL/STL/CxImage����������
#define MSVC_NO_WARNING_AREA_BEGIN                      \
	__pragma(warning(push, 1))                        \
	__pragma(warning(disable: 4018 4100 4127 4191 4201 4204 4242 4244 4245 4290 4365))        \
	__pragma(warning(disable: 4505 4548 4555 4701 4702 4706 4819 4917 4920 4995 4996))        \
	__pragma(warning(disable: 6011 6031 6053 6202 6211 6246 6248 6255 6269 6282 6308 6323 6328 6334 6385 6386 6387 6400 6401))

#define MSVC_NO_WARNING_AREA_END                        \
	__pragma(warning(pop))

// ���������ջ�ĺ꣬��stdafx.h���á�
// ĳЩͷ�ļ�(��boost/regex_token_iterator.hpp)���ܶԾ����ջ��ʹ�úܲ�С�ģ�
// push��pop�٣�����������һ��������ٵ�ջ֡�У���������Ϊ��������
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


// ��boost1.34�У�����boost/regex.hpp�����pragma warning��ջ��ƽ����������PUSH��
// ������cpp���������boost/regex.hpp�����ڰ���֮��д������꣬����ܶ�warning��boost���Ե���
#define MSVC_WARNING_WORKAROUND__BOOST_1_34_REGEX __pragma(warning(pop)) __pragma(warning(pop))

// ������VCȱʡ�رյľ���
// ����������ڹ���ѡ����ָ���������/Wall��������©ĳЩ����
// �رղ����ĵľ���
#define MSVC_TUNE_WARNINGS \
__pragma(warning(default: 4061 4062 4191 4242 4254 4263 4264 4265 4266 4287 4296)) \
__pragma(warning(default: 4302 4365 4514 4545 4546 4547 4548 4549 4555 4571)) \
__pragma(warning(default: 4623 4625 4626 4640 4668 4682 4710 4711 4820 4826 4836)) \
__pragma(warning(default: 4905 4906 4917 4928 4946)) \
\
__pragma(warning(disable: 4061 4127 4503 4514 4571 4623 4625 4626 4710 4820)) \
__pragma(warning(disable: 4505 4512))

// 4061) ��switchһ��enumʱû����case����ʽָ�����е�enum�ж������ֵ
// 4127) �������ʽ��һ������(��ATL_ENSURE,ATL_ENSURE_RETUEN_VAL���õ�)
// 4355) thisָ�����ڳ�ʼ���б����Ŀ���õ÷ǳ���
// 4503) ģ��չ����ı�ʶ������
// 4514) û���õ��������������Ż���
// 4623 4625 4626) ��Ϊ���๹�캯��/�������캯��/��ֵ���������ɼ�,�����޷�Ϊ��������Ĭ�ϵ�
// 4820) ����(�ṹ)�����ڳ�Ա�������padding
// 4505) δ�����õ��ĺ������Ż���
// 4512) ȱʡ�ĸ�ֵ���������ܱ�����


#define MSVC_CONFIGURE_WARNINGS \
	MSVC_CLEAR_WARNING_STACK \
	MSVC_TUNE_WARNINGS

// ������ʩ

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
