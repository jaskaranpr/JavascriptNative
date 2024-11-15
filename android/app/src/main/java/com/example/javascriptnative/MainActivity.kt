package com.example.javascriptnative

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.javascriptnative.skia.SkiaView

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(SkiaView(this))
    }
}