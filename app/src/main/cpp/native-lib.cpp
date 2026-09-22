#include <jni.h>
#include <string>
#include <android/log.h>
#include <bytehook.h>
#include <cstdarg>
#include <cstdio>

#define LOG_TAG "ByteHookDemo"

// Kotlin 先调用这个方法，用来确认 libjiagu_native.so 已经成功加载。
extern "C" JNIEXPORT jstring JNICALL
Java_com_smartclean_jiagu_MainActivity_nativeString(JNIEnv *env, jobject /* thiz */) {
    std::string message = "C++ native library is ready";
    return env->NewStringUTF(message.c_str());
}

// 保存 hook 任务句柄，避免重复安装同一个 hook，也方便后续取消 hook。
static bytehook_stub_t g_printf_stub = nullptr;

// printf 的代理函数。ByteHook 会把 libc.so 中的 printf 调用重定向到这里，
// 所以这个函数的签名必须和原始 printf 兼容。
static int proxy_printf(const char *format, ...) {
    // 自动模式下必须加这一句，用来让 ByteHook 维护调用栈，并在函数返回前安全清理。
    BYTEHOOK_STACK_SCOPE();

    char buffer[1024];

    // printf 是可变参数函数，所以这里先用 va_list 读取原始参数，
    // 方便打印日志，也方便后面转发给原始 printf。
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

    // 调用上一个/原始 printf。在 C++ 中 BYTEHOOK_CALL_PREV 会直接发起调用，
    // 不要再把它强转成函数指针。
    return BYTEHOOK_CALL_PREV(proxy_printf, "[hooked] %s", buffer);
}

// 点击按钮时由 Kotlin 调用。这里安装一个全局 PLT hook，
// 目标是所有调用到 libc.so 中 printf 的地方。
extern "C" JNIEXPORT jstring JNICALL
Java_com_smartclean_jiagu_MainActivity_installPrintfHook(JNIEnv *env, jobject /* thiz */) {
    if (g_printf_stub != nullptr) {
        return env->NewStringUTF("printf hook already installed");
    }

    // hook_all 会处理当前已加载以及后续加载的调用方库，只要它们导入的是 libc 的 printf。
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

// 主动触发一次 native 层 printf，方便从 logcat 观察 proxy_printf 是否被命中。
extern "C" JNIEXPORT jstring JNICALL
Java_com_smartclean_jiagu_MainActivity_testPrintfHook(JNIEnv *env, jobject /* thiz */) {
    int ret = printf("printf from native-lib.cpp\n");
    std::string message = "native printf returned " + std::to_string(ret);
    return env->NewStringUTF(message.c_str());
}

// 可选的清理入口。当前 Activity 暂时没有调用它，
// 但调试安装/卸载 hook 行为时会很方便。
extern "C" JNIEXPORT void JNICALL
Java_com_smartclean_jiagu_MainActivity_uninstallPrintfHook(JNIEnv * /* env */, jobject /* thiz */) {
    if (g_printf_stub != nullptr) {
        bytehook_unhook(g_printf_stub);
        g_printf_stub = nullptr;
    }
}
