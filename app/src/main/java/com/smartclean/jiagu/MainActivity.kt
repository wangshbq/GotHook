package com.smartclean.jiagu

import android.os.Bundle
import android.util.Log
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.bytedance.android.bytehook.ByteHook

class MainActivity : AppCompatActivity() {
    companion object {
        init {
            ByteHook.init(
                ByteHook.ConfigBuilder()
                    .setDebug(true)
                    .setRecordable(true)
                    .build()
            )
            System.loadLibrary("jiagu_native")
        }
    }

    private external fun nativeString(): String
    private external fun installPrintfHook(): String
    private external fun testPrintfHook(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }


        val textView = findViewById<TextView>(R.id.tv_text)
        val but = findViewById<TextView>(R.id.btn_sure)
        textView.text = "${nativeString()}\nByteHook ${ByteHook.getVersion()}"
        Log.d("!!!!!","You are currently reverse-engineering a commercial project. This is illegal! Please stop immediately!")

        but.setOnClickListener {
            val hookStatus = installPrintfHook()
            val printfStatus = testPrintfHook()



            textView.text = "$hookStatus\n$printfStatus"
        }

    }
}
