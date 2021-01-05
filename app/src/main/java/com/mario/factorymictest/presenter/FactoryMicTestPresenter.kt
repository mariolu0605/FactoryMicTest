package com.mario.factorymictest.presenter

import android.app.Activity
import android.media.MediaPlayer
import android.util.Log
import android.view.View
import com.mario.factorymictest.R
import com.mario.factorymictest.jni.FactoryMicTestJNI
import com.mario.factorymictest.view.IView

class FactoryMicTestPresenter(view: IView) {

    var playCount = 0

    var mediaPlayerInconsistent: MediaPlayer = MediaPlayer.create(view as Activity, R.raw.sin)

    var mediaPlayerWakeup: MediaPlayer = MediaPlayer.create(view as Activity, R.raw.xiaot).apply {
        setOnCompletionListener {
            Log.d("MarioLu", "over")
            if (++playCount < 3) {
                start()
            } else {
                mView.notifyChangeWakeupResult(false)
            }
        }
    }


    private val mView: IView = view

    init {

        FactoryMicTestJNI.initJNI()
        Log.d("MarioLu", "init")
        FactoryMicTestJNI.addBDSpiMsgListener(object : IBDSpiReturnMsgListener {
            override fun returnMsg(msg: String) {
                Log.d("MarioLu", "returnMsg = $msg")
                notifyView(msg)
            }
        })

    }

    fun wakeupTest() {
        FactoryMicTestJNI.wakeUpTest()
        Log.e("Mario", "mediaPlayer.isPlaying  = ${mediaPlayerWakeup.isPlaying}")

        if (!mediaPlayerWakeup.isPlaying) {
            mediaPlayerWakeup.start()
        }
    }

    fun inconsistentTest() {
        if (!mediaPlayerInconsistent.isPlaying) {
            mediaPlayerInconsistent.start()
        }
        FactoryMicTestJNI.inconsistentTest()

    }

    fun release() {
        mediaPlayerRelease()
        FactoryMicTestJNI.release()
        Thread().run()
    }


    private fun mediaPlayerRelease() {
        if (mediaPlayerInconsistent != null) {
            mediaPlayerInconsistent.stop()
            mediaPlayerInconsistent.release()
        }
        if (mediaPlayerWakeup != null) {
            mediaPlayerWakeup.stop()
            mediaPlayerWakeup.release()
        }
    }

    fun notifyView(msg: String) {
        if (msg == "wakeup") {
            mView.notifyChangeWakeupResult(true)
        }

        if (msg.contains("inconsistent")) {
            val subMsg = msg.substring(13)

            val status = intArrayOf(-1,-1,-1,-1,-1,-1)
            val energy = floatArrayOf(0f,0f,0f,0f,0f,0f)

            Log.d("MarioLu", subMsg.split(",").toString())
            for (i in 0..subMsg.split(",").size - 2) {

                status[i] = subMsg.split(",")[i].split("&&")[0].toInt()
                energy[i] = subMsg.split(",")[i].split("&&")[1].toFloat()
            }




            status.forEach {
                Log.d("MarioLu", it.toString()+"44")
            }

            energy.forEach {
                Log.d("MarioLu", it.toString())
            }


            mView.notifyChangeInconsistentResult(status,energy,true)


        }

    }

}