package com.example.javascriptnative.skia

import android.content.Context
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.graphics.PixelFormat

class SkiaWrapper {
    external fun nativeDraw(surface: Surface)

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

class SkiaView(context: Context) : SurfaceView(context), SurfaceHolder.Callback {
    private val skiaWrapper = SkiaWrapper()
    private var isSurfaceValid = false

    init {
        holder.addCallback(this)
        // Set pixel format explicitly
        holder.setFormat(PixelFormat.RGBA_8888)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        Log.i("SkiaView", "Surface created")
        isSurfaceValid = true
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        Log.i("SkiaView", "Surface changed: ${width}x${height}, format: $format")

        if (!isSurfaceValid) {
            Log.e("SkiaView", "Surface is not valid")
            return
        }

        if (width <= 0 || height <= 0) {
            Log.e("SkiaView", "Invalid dimensions")
            return
        }

        val surface = holder.surface
        if (surface == null || !surface.isValid) {
            Log.e("SkiaView", "Invalid surface")
            return
        }

        try {
            skiaWrapper.nativeDraw(surface)
        } catch (e: Exception) {
            Log.e("SkiaView", "Drawing failed", e)
        }
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        Log.i("SkiaView", "Surface destroyed")
        isSurfaceValid = false
    }
}