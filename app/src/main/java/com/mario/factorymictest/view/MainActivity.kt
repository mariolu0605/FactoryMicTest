package com.mario.factorymictest.view

import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.app.Activity
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.annotation.RequiresApi
import com.mario.factorymictest.R
import com.mario.factorymictest.presenter.FactoryMicTestPresenter
import kotlinx.android.synthetic.main.activity_main.*
import java.util.concurrent.Executors
import kotlin.concurrent.thread
import kotlin.system.exitProcess

class MainActivity : Activity(), IView {


    private lateinit var presenter: FactoryMicTestPresenter
    private val wakeButtonAnimatorSet = AnimatorSet()
    private val inconsistentButtonAnimatorSet = AnimatorSet()

    private lateinit var wakeButtonScaleXAnimator: ObjectAnimator
    private lateinit var wakeButtonScaleYAnimator: ObjectAnimator

    private lateinit var inconsistentButtonScaleXAnimator: ObjectAnimator
    private lateinit var inconsistentButtonScaleYAnimator: ObjectAnimator


    @RequiresApi(Build.VERSION_CODES.O)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        presenter = FactoryMicTestPresenter(this)


        button_wake.setOnClickListener {
            notifyChangeWakeupTextView("正在进行唤醒测试")
            notifyChangeWakeupTextColor(Color.WHITE)
            thread {
                presenter.wakeupTest()
            }
        }

        button_inconsistent.setOnClickListener {
            notifyChangeInconsistentTextView("正在进行麦克风一致性测试")
            thread {
                presenter.inconsistentTest()
            }

        }

        button_wake.setOnFocusChangeListener { v, hasFocus ->
            if (hasFocus) {
                wakeButtonScaleXAnimator = ObjectAnimator.ofFloat(v, View.SCALE_X, 1f, 1.12f)
                wakeButtonScaleYAnimator = ObjectAnimator.ofFloat(v, View.SCALE_Y, 1f, 1.12f)
                wakeButtonAnimatorSet.playTogether(
                    wakeButtonScaleXAnimator,
                    wakeButtonScaleYAnimator
                )
                wakeButtonAnimatorSet.start()
            } else {
                wakeButtonAnimatorSet.reverse()
            }
        }


        button_inconsistent.setOnFocusChangeListener { v, hasFocus ->
            if (hasFocus) {
                inconsistentButtonScaleXAnimator =
                    ObjectAnimator.ofFloat(v, View.SCALE_X, 1f, 1.12f)
                inconsistentButtonScaleYAnimator =
                    ObjectAnimator.ofFloat(v, View.SCALE_Y, 1f, 1.12f)
                inconsistentButtonAnimatorSet.playTogether(
                    inconsistentButtonScaleXAnimator,
                    inconsistentButtonScaleYAnimator
                )
                inconsistentButtonAnimatorSet.start()
            } else {
                inconsistentButtonAnimatorSet.reverse()
            }
        }
    }


    override fun onStop() {
        presenter.release()
        super.onStop()
        exitProcess(0)
    }


    override fun notifyChangeWakeupResult(result: Boolean) {

        if (presenter.mediaPlayerWakeup.isPlaying) {
            presenter.mediaPlayerWakeup.pause()
        }
        presenter.playCount = 0
        this.runOnUiThread {
            notifyChangeWakeupTextView(if (result) "唤醒测试通过" else "唤醒测试未通过")
            textView_wakeup.setTextColor(if (result) Color.GREEN else Color.RED)
        }
    }

    override fun notifyChangeInconsistentResult(
        status: IntArray,
        energy: FloatArray,
        result: Boolean
    ) {
        if (presenter.mediaPlayerInconsistent.isPlaying) {
            presenter.mediaPlayerInconsistent.pause()
        }

        var inconsistentResult = ""
        for (i in status.indices) {
            inconsistentResult += if (i < 4) {
                "Mic[$i] = ${status[i]}  energy = ${energy[i]} \n"
            } else {
                "Ref[$i] = ${status[i]}  energy = ${energy[i]} \n"
            }

        }
        this.runOnUiThread {
            textView_inconsistent.text = inconsistentResult
        }

    }

    override fun notifyChangeWakeupTextView(content: String) {
        textView_wakeup.text = content
    }

    override fun notifyChangeInconsistentTextView(content: String) {
        textView_inconsistent.text = content
    }

    override fun notifyChangeWakeupTextColor(color: Int) {
        textView_wakeup.setTextColor(color)
    }


}
