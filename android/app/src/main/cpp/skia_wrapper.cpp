#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkBitmap.h"

#define LOG_TAG "SkiaDrawing"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
Java_com_example_javascriptnative_skia_SkiaWrapper_nativeDraw(
        JNIEnv* env, jobject /* this */, jobject surface) {

    // Obtain the native window from the Java Surface
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGE("Could not get native window");
        return;
    }

    int32_t width = ANativeWindow_getWidth(window);
    int32_t height = ANativeWindow_getHeight(window);
    LOGI("Window dimensions: width=%d, height=%d", width, height);

    // Configure the native window buffer format
    if (ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888) != 0) {
        LOGE("Failed to set buffer geometry");
        ANativeWindow_release(window);
        return;
    }

    // Lock the buffer for drawing
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, nullptr) < 0) {
        LOGE("Failed to lock native window buffer");
        ANativeWindow_release(window);
        return;
    }

    // Set up Skia Bitmap backed by the native buffer
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(width, height);
    if (!bitmap.installPixels(imageInfo, buffer.bits, buffer.stride * 4)) {
        LOGE("Failed to install pixels on SkBitmap");
        ANativeWindow_unlockAndPost(window);
        ANativeWindow_release(window);
        return;
    }

    // Create a canvas to draw on the bitmap
    SkCanvas canvas(bitmap);

    // Clear the canvas with white
    SkPaint bgPaint;
    bgPaint.setColor(SK_ColorWHITE);
    canvas.drawPaint(bgPaint);

    // Draw a blue circle in the center
    SkPaint circlePaint;
    circlePaint.setColor(SK_ColorBLUE);
    circlePaint.setAntiAlias(true);
    canvas.drawCircle(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 100.0f, circlePaint);

    // Finalize and post buffer
    ANativeWindow_unlockAndPost(window);
    ANativeWindow_release(window);
    LOGI("Drawing complete");
}
