package com.example.javascriptnative.skia

import android.content.Context
import android.content.res.AssetManager
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.graphics.PixelFormat

class SkiaWrapper {
    external fun nativeDrawWithJS(surface: Surface, assetManager: AssetManager, jsFileName: String)

    companion object {
        init {
            try {
                System.loadLibrary("skia_wrapper")
                Log.i("SkiaWrapper", "Library loaded successfully")
            } catch (e: UnsatisfiedLinkError) {
                Log.e("SkiaWrapper", "Failed to load library", e)
            }
        }
    }
}

class SkiaView @JvmOverloads constructor(
    context: Context,
    private var jsFileName: String = "app.js"
) : SurfaceView(context), SurfaceHolder.Callback {

    private val skiaWrapper = SkiaWrapper()
    private var isSurfaceValid = false
    private var isDrawing = false

    init {
        holder.addCallback(this)
        holder.setFormat(PixelFormat.RGBA_8888)
    }

    fun setJavaScriptFile(fileName: String) {
        jsFileName = fileName
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
                    skiaWrapper.nativeDrawWithJS(surface, context.assets, jsFileName)
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
    }
}