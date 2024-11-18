#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
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

static JSValue js_drawText(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 3 || argc > 5) return JS_EXCEPTION; // At least text, x, and y are required

    auto* drawCtx = static_cast<DrawingContext*>(JS_GetContextOpaque(ctx));

    // Extract text, x, and y
    const char* text;
    size_t textLen;
    text = JS_ToCStringLen(ctx, &textLen, argv[0]);
    if (!text) return JS_EXCEPTION;

    double x, y;
    JS_ToFloat64(ctx, &x, argv[1]);
    JS_ToFloat64(ctx, &y, argv[2]);

    // Optional parameters: font size and color
    double fontSize = 16.0; // Default font size
    SkColor color = SK_ColorBLACK; // Default color
    if (argc > 3) JS_ToFloat64(ctx, &fontSize, argv[3]);
    if (argc > 4) {
        uint32_t colorValue;
        if (JS_ToUint32(ctx, &colorValue, argv[4]) == -1) {
            JS_FreeCString(ctx, text);
            return JS_EXCEPTION;
        }
        color = static_cast<SkColor>(colorValue);
    }

    // Configure the paint object
    SkPaint paint = drawCtx->paint; // Use the existing paint object
    paint.setAntiAlias(true);
    paint.setColor(color);

    // Configure the font
    SkFont font;
    font.setSize(fontSize);

    // Render the text
    drawCtx->canvas->drawString(text, x, y, font, paint);

    // Free allocated text string
    JS_FreeCString(ctx, text);

    return JS_UNDEFINED;
}



extern "C" JNIEXPORT void JNICALL Java_com_example_javascriptnative_skia_SkiaWrapper_nativeDrawWithJS(
        JNIEnv* env, jobject thiz, jobject surface, jobject assetManager, jstring jsFileName) {

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
    ANativeWindow_Buffer windowBuffer;  // Renamed from buffer to windowBuffer
    if (!bufferGuard.lock(windowBuffer)) {
    LOGE("Failed to lock native window buffer");
    return;
    }

    // Set up Skia Bitmap backed by the native buffer
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(width, height);
    if (!bitmap.installPixels(imageInfo, windowBuffer.bits, windowBuffer.stride * 4)) {
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
    js.addFunction("drawText", js_drawText, 5);

    // Add color constants
    js.addConstant("RED", SK_ColorRED);
    js.addConstant("GREEN", SK_ColorGREEN);
    js.addConstant("BLUE", SK_ColorBLUE);
    js.addConstant("BLACK", SK_ColorBLACK);
    js.addConstant("WHITE", SK_ColorWHITE);
    js.addConstant("SCREEN_WIDTH", width);
    js.addConstant("SCREEN_HEIGHT", height);

    // Get JavaScript code from asset file
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
    LOGE("Could not get asset manager");
    return;
    }

    const char* filename = env->GetStringUTFChars(jsFileName, nullptr);
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
    env->ReleaseStringUTFChars(jsFileName, filename);

    if (!asset) {
    LOGE("Could not open asset");
    return;
    }

    // Read the JavaScript file content
    long size = AAsset_getLength(asset);
    char* jsContentBuffer = new char[size + 1];  // Renamed from buffer to jsContentBuffer
    AAsset_read(asset, jsContentBuffer, size);
    jsContentBuffer[size] = '\0';
    AAsset_close(asset);

    // Execute JavaScript
    std::string error;
    if (!js.evaluate(jsContentBuffer, &error)) {
    LOGE("JavaScript error: %s", error.c_str());
    }

    delete[] jsContentBuffer;

    // Copy pixels back to buffer
    memcpy(windowBuffer.bits, bitmap.getPixels(), bitmap.computeByteSize());

    LOGI("Drawing complete");
}