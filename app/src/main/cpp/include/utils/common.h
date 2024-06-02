#pragma once

#include <jni.h>

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define CONNECT_FAILED 101
#define INIT_FAILED 102

void throwNativeError(JNIEnv *env, int code);
