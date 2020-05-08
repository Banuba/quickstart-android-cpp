package com.banuba.sdk.example.quickstart_cpp

import java.nio.ByteBuffer

class BanubaSdk {
    external fun initialize(pathToResources: String, clientToken: String)
    external fun deinitialize()

    external fun createEffectPlayer(): Long
    external fun destroyEffectPlayer(effectPlayer: Long)

    external fun surfaceCreated(effectPlayer: Long, width: Int, height: Int)
    external fun surfaceChanged(effectPlayer: Long, width: Int, height: Int)
    external fun surfaceDestroyed(effectPlayer: Long)

    external fun loadEffect(effectPlayer: Long, name: String)
    external fun processPhoto(effectPlayer: Long, rgba: ByteBuffer, width: Int, height: Int): ByteArray


    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}