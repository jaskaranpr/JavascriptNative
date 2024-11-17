package com.example.javascriptnative

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.javascriptnative.skia.SkiaView

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val skiaView = SkiaView(this)
        setContentView(skiaView)

        // Example: Draw some shapes
        skiaView.setJavaScriptCode("""
            // Clear screen
            setColor(WHITE);
            drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            
            // Draw a blue circle
            setColor(BLUE);
            drawCircle(250, 250, 100);
            
            // Draw a red rectangle
            setColor(RED);
            drawRect(100, 100, 150, 150);
            
            // Draw a green circle
            setColor(GREEN);
            drawCircle(400, 400, 80);
        """)
    }
}