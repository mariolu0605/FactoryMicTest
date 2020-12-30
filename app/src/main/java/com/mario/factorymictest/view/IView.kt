package com.mario.factorymictest.view

interface IView {

    fun notifyChangeWakeupResult(result: Boolean)


    fun notifyChangeInconsistentResult(status:IntArray,energy:FloatArray,result: Boolean)


    fun notifyChangeWakeupTextView(content : String)


    fun notifyChangeInconsistentTextView(content : String)
}