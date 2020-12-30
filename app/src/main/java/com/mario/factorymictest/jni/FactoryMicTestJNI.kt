package com.mario.factorymictest.jni

import android.util.Log
import com.mario.factorymictest.presenter.IBDSpiReturnMsgListener


private const val WAKEUP = 1
private const val INCONSISTENT = 2

object FactoryMicTestJNI {

    private var bDSpiReturnMsgListeners = ArrayList<IBDSpiReturnMsgListener>()


    init {
        System.loadLibrary("bds_audio_client")
        System.loadLibrary("bdMicDetect")
        System.loadLibrary("Test")
        System.loadLibrary("native-lib")
    }


    external fun stringFromJNI(): String

    external fun wakeUpTest(): Int


    private external fun wakeUpStop()

    external fun release()

    external fun initJNI()

    external fun inconsistentTest()


    // Used to load the 'native-lib' library on application startup.

    @JvmStatic
    fun _bdspiCallBack(msg: String) {
        when (msg.split("]")[0].toInt()) {
            WAKEUP -> {
                Log.e("MarioLu", "wakeup")
                sendMsg("wakeup")


                wakeUpStop()
            }
            INCONSISTENT -> {
                sendMsg("inconsistent_"+msg.substring(2))
                Log.e("MarioLu", "inconsistent_"+msg.substring(2))
                sendMsg(msg)
            }
        }
    }

    fun addBDSpiMsgListener(bdSpiReturnMsg: IBDSpiReturnMsgListener) {
        Log.d("MarioLu", "add callback")
        bDSpiReturnMsgListeners.add(bdSpiReturnMsg)
    }

    fun removeBDSpiMsgListener(bdSpiReturnMsg: IBDSpiReturnMsgListener) {
        bDSpiReturnMsgListeners.remove(bdSpiReturnMsg)
    }

    private fun sendMsg(msg: String) {
        bDSpiReturnMsgListeners.forEach {
            it.returnMsg(msg)
        }
    }


}