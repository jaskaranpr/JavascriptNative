#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <cstring>

#define LOG_TAG "SkiaWrapper"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Color helper functions
inline uint32_t makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_javascriptnative_skia_SkiaWrapper_nativeDraw(
        JNIEnv* env, jobject /* this */, jobject surface) {

    if (!env || !surface) {
        LOGE("Invalid input parameters");
        return;
    }

    LOGI("Starting native draw");

    // Get native window
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGE("Failed to get native window");
        return;
    }

    // Configure the buffer format
    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);

    // Get window dimensions
    int32_t width = ANativeWindow_getWidth(window);
    int32_t height = ANativeWindow_getHeight(window);

    if (width <= 0 || height <= 0) {
        LOGE("Invalid dimensions: %dx%d", width, height);
        ANativeWindow_release(window);
        return;
    }

    LOGI("Window dimensions: %dx%d", width, height);

    // Lock buffer
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, nullptr) != 0) {
        LOGE("Failed to lock window buffer");
        ANativeWindow_release(window);
        return;
    }

    // Verify buffer
    if (!buffer.bits) {
        LOGE("Null buffer bits");
        ANativeWindow_unlockAndPost(window);
        ANativeWindow_release(window);
        return;
    }

    // Fill buffer with a test pattern
    uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
    int32_t stride = buffer.stride;

    for (int32_t y = 0; y < buffer.height; y++) {
        for (int32_t x = 0; x < buffer.width; x++) {
            // Create a gradient pattern
            uint8_t r = static_cast<uint8_t>((x * 255) / buffer.width);
            uint8_t g = static_cast<uint8_t>((y * 255) / buffer.height);
            uint8_t b = 255;

            // Draw a white border
            if (x < 2 || x >= buffer.width - 2 ||
                y < 2 || y >= buffer.height - 2) {
                pixels[y * stride + x] = makeColor(255, 255, 255); // White
            }
                // Draw a central rectangle
            else if (x > width/4 && x < 3*width/4 &&
                     y > height/4 && y < 3*height/4) {
                pixels[y * stride + x] = makeColor(0, 0, 255); // Blue
            }
                // Fill the rest with the gradient
            else {
                pixels[y * stride + x] = makeColor(r, g, b);
            }
        }
    }

    LOGI("Buffer filled successfully");

    // Unlock and cleanup
    ANativeWindow_unlockAndPost(window);
    ANativeWindow_release(window);
    LOGI("Drawing completed");
}