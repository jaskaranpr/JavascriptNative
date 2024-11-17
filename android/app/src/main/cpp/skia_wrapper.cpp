#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "quickjs_wrapper.h"

#define LOG_TAG "SkiaJS"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// RAII helpers
class ScopedWindow {
public:
    explicit ScopedWindow(ANativeWindow* w) : window(w) {}
    ~ScopedWindow() { if (window) ANativeWindow_release(window); }
    ANativeWindow* get() { return window; }
private:
    ANativeWindow* window;
};

class ScopedBuffer {
public:
    ScopedBuffer(ANativeWindow* w) : window(w), locked(false) {}
    ~ScopedBuffer() { if (locked) ANativeWindow_unlockAndPost(window); }

    bool lock(ANativeWindow_Buffer& buffer) {
        if (ANativeWindow_lock(window, &buffer, nullptr) < 0) return false;
        locked = true;
        return true;
    }
private:
    ANativeWindow* window;
    bool locked;
};

// Global drawing context
struct DrawingContext {
    SkCanvas* canvas;
    SkPaint paint;

    DrawingContext() {
        paint.setAntiAlias(true);
    }
};

// JS drawing functions
static JSValue js_setColor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc != 1) return JS_EXCEPTION;

    auto* drawCtx = static_cast<DrawingContext*>(JS_GetContextOpaque(ctx));
    uint32_t color;
    JS_ToUint32(ctx, &color, argv[0]);
    drawCtx->paint.setColor(color);

    return JS_UNDEFINED;
}

static JSValue js_drawCircle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc != 3) return JS_EXCEPTION;

    auto* drawCtx = static_cast<DrawingContext*>(JS_GetContextOpaque(ctx));
    double x, y, radius;
    JS_ToFloat64(ctx, &x, argv[0]);
    JS_ToFloat64(ctx, &y, argv[1]);
    JS_ToFloat64(ctx, &radius, argv[2]);

    drawCtx->canvas->drawCircle(x, y, radius, drawCtx->paint);
    return JS_UNDEFINED;
}

static JSValue js_drawRect(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc != 4) return JS_EXCEPTION;

    auto* drawCtx = static_cast<DrawingContext*>(JS_GetContextOpaque(ctx));
    double x, y, w, h;
    JS_ToFloat64(ctx, &x, argv[0]);
    JS_ToFloat64(ctx, &y, argv[1]);
    JS_ToFloat64(ctx, &w, argv[2]);
    JS_ToFloat64(ctx, &h, argv[3]);

    drawCtx->canvas->drawRect(SkRect::MakeXYWH(x, y, w, h), drawCtx->paint);
    return JS_UNDEFINED;
}

extern "C" JNIEXPORT void JNICALL Java_com_example_javascriptnative_skia_SkiaWrapper_nativeDrawWithJS(
        JNIEnv* env, jobject /* this */, jobject surface, jstring jsCode) {

    // Get native window with RAII
    ScopedWindow window(ANativeWindow_fromSurface(env, surface));
    if (!window.get()) {
    LOGE("Could not get native window");
    return;
    }

    const int32_t width = ANativeWindow_getWidth(window.get());
    const int32_t height = ANativeWindow_getHeight(window.get());
    LOGI("Window dimensions: width=%d, height=%d", width, height);

    // Configure the native window buffer format
    if (ANativeWindow_setBuffersGeometry(window.get(), width, height, WINDOW_FORMAT_RGBA_8888) != 0) {
    LOGE("Failed to set buffer geometry");
    return;
    }

    // Create buffer with RAII
    ScopedBuffer bufferGuard(window.get());
    ANativeWindow_Buffer buffer;
    if (!bufferGuard.lock(buffer)) {
    LOGE("Failed to lock native window buffer");
    return;
    }

    // Set up Skia Bitmap backed by the native buffer
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(width, height);
    if (!bitmap.installPixels(imageInfo, buffer.bits, buffer.stride * 4)) {
    LOGE("Failed to install pixels on SkBitmap");
    return;
    }

    // Create a canvas to draw on the bitmap
    SkCanvas canvas(bitmap);

    // Set up drawing context
    DrawingContext drawCtx;
    drawCtx.canvas = &canvas;

    // Set up QuickJS
    QuickJSWrapper js;
    JS_SetContextOpaque(js.context(), &drawCtx);

    // Register drawing functions
    js.addFunction("setColor", js_setColor, 1);
    js.addFunction("drawCircle", js_drawCircle, 3);
    js.addFunction("drawRect", js_drawRect, 4);

    // Add color constants
    js.addConstant("RED", SK_ColorRED);
    js.addConstant("GREEN", SK_ColorGREEN);
    js.addConstant("BLUE", SK_ColorBLUE);
    js.addConstant("BLACK", SK_ColorBLACK);
    js.addConstant("WHITE", SK_ColorWHITE);
    js.addConstant("SCREEN_WIDTH", width);
    js.addConstant("SCREEN_HEIGHT", height);

    // Execute JavaScript with proper cleanup
    const char* code = env->GetStringUTFChars(jsCode, nullptr);
    std::string error;
    if (!js.evaluate(code, &error)) {
    LOGE("JavaScript error: %s", error.c_str());
    }
    env->ReleaseStringUTFChars(jsCode, code);

    // Copy pixels back to buffer
    memcpy(buffer.bits, bitmap.getPixels(), bitmap.computeByteSize());

    LOGI("Drawing complete");
}