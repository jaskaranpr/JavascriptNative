package com.example.javascriptnative.skia

import android.content.Context
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.graphics.PixelFormat

class SkiaWrapper {
    external fun nativeDrawWithJS(surface: Surface, jsCode: String)

    companion object {
        init {
            try {
                System.loadLibrary("skia_wrapper")
                Log.i("SkiaWrapper", "Library loaded successfully")
            } catch (e: UnsatisfiedLinkError) {
                Log.e("SkiaWrapper", "Failed to load library", e)
            }
        }

        // Default JavaScript drawing code
        const val DEFAULT_JS_CODE = """
            // Clear background to white
            setColor(WHITE);
            drawRect(0, 0, 1000, 2000);
            
            // Draw some shapes
            setColor(BLUE);
            drawCircle(250, 250, 100);
            
            setColor(RED);
            drawRect(100, 100, 150, 150);
        """
    }
}

class SkiaView @JvmOverloads constructor(
    context: Context,
    private var jsCode: String = SkiaWrapper.DEFAULT_JS_CODE
) : SurfaceView(context), SurfaceHolder.Callback {

    private val skiaWrapper = SkiaWrapper()
    private var isSurfaceValid = false
    private var isDrawing = false

    init {
        holder.addCallback(this)
        holder.setFormat(PixelFormat.RGBA_8888)
    }

    fun setJavaScriptCode(code: String) {
        jsCode = code
        if (isSurfaceValid && !isDrawing) {
            requestDraw()
        }
    }

    private fun requestDraw() {
        if (!isSurfaceValid || isDrawing) return

        holder.surface?.let { surface ->
            if (surface.isValid) {
                try {
                    isDrawing = true
                    skiaWrapper.nativeDrawWithJS(surface, jsCode)
                } catch (e: Exception) {
                    Log.e(TAG, "Drawing failed", e)
                } finally {
                    isDrawing = false
                }
            }
        }
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        Log.i(TAG, "Surface created")
        isSurfaceValid = true
        requestDraw()
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        Log.i(TAG, "Surface changed: ${width}x${height}, format: $format")

        if (!isSurfaceValid) {
            Log.e(TAG, "Surface is not valid")
            return
        }

        if (width <= 0 || height <= 0) {
            Log.e(TAG, "Invalid dimensions")
            return
        }

        requestDraw()
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        Log.i(TAG, "Surface destroyed")
        isSurfaceValid = false
        isDrawing = false
    }

    companion object {
        private const val TAG = "SkiaView"

        // JavaScript color constants
        const val RED = "RED"
        const val GREEN = "GREEN"
        const val BLUE = "BLUE"
        const val BLACK = "BLACK"
        const val WHITE = "WHITE"
    }
}