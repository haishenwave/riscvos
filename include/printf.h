//printf 函数声明,让其他文件知道如何调用

#pragma once
#include <stdarg.h>//处理可变函数（就是...)
/*
*printf函数声明
*fmt：格式字符串
*...: 可变参数,数量不固定
*/

int printf(const char *fmt,...);