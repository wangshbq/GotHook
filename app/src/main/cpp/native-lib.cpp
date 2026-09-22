#include <jni.h>
#include <string>
#include <android/log.h>
#include <bytehook.h>
#include <cstdarg>
#include <cstdio>

#define LOG_TAG "ByteHookDemo"

extern "C" JNIEXPORT jstring JNICALL
Java_com_smartclean_jiagu_MainActivity_nativeString(JNIEnv *env, jobject /* thiz */) {
    std::string message = "C++ native library is ready";
    return env->NewStringUTF(message.c_str());
}

static bytehook_stub_t g_printf_stub = nullptr;

static int proxy_printf(const char *format, ...) {
    BYTEHOOK_STACK_SCOPE();

    char buffer[1024];

    va_list args;
    va_start(args, format);
    int ret = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    __android_log_print(
            ANDROID_LOG_INFO,
            LOG_TAG,
            "printf hooked: %s",
            buffer
    );

    return BYTEHOOK_CALL_PREV(proxy_printf, "[hooked] %s", buffer);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_smartclean_jiagu_MainActivity_installPrintfHook(JNIEnv *env, jobject /* thiz */) {
    if (g_printf_stub != nullptr) {
        return env->NewStringUTF("printf hook already installed");
    }

    g_printf_stub = bytehook_hook_all(
            "libc.so",
            "printf",
            reinterpret_cast<void *>(proxy_printf),
            nullptr,
            nullptr
    );

    if (g_printf_stub == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "hook printf failed");
        return env->NewStringUTF("hook printf failed");
    } else {
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "hook printf success");
        return env->NewStringUTF("hook printf success");
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_smartclean_jiagu_MainActivity_testPrintfHook(JNIEnv *env, jobject /* thiz */) {
    int ret = printf("printf from native-lib.cpp\n");
    std::string message = "native printf returned " + std::to_string(ret);
    return env->NewStringUTF(message.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_smartclean_jiagu_MainActivity_uninstallPrintfHook(JNIEnv * /* env */, jobject /* thiz */) {
    if (g_printf_stub != nullptr) {
        bytehook_unhook(g_printf_stub);
        g_printf_stub = nullptr;
    }
}
