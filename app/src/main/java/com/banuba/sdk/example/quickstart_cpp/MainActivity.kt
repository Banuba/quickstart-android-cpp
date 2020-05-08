package com.banuba.sdk.example.quickstart_cpp

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.os.ProxyFileDescriptorCallback
import androidx.appcompat.app.AppCompatActivity
import com.banuba.sdk.example.common.BANUBA_CLIENT_TOKEN
import com.banuba.sdk.utils.ContextProvider
import com.banuba.utils.FileUtilsNN
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File
import java.nio.ByteBuffer
import java.util.zip.ZipFile
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class MainActivity : AppCompatActivity() {

    var banubaSdk = BanubaSdk()

    lateinit var glView: GLSurfaceView
    lateinit var render: EPRenderer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val resPath = application.filesDir.absolutePath + "/bnb-resources"
        if (!File(resPath).exists()) {
            unzipResources(resPath)
            copyEffects(resPath)
        }

        ContextProvider.setContext(applicationContext)

        FileUtilsNN.setContext(applicationContext)
        FileUtilsNN.setResourcesBasePath(resPath)

        banubaSdk.initialize(resPath, BANUBA_CLIENT_TOKEN)

        render = EPRenderer(banubaSdk) { result ->
            runOnUiThread {
                // show processing result
                setContentView(R.layout.activity_main)
                imageView.setImageBitmap(result)
            }
        }

        glView = GLSurfaceView(this)
        glView.setEGLContextClientVersion(3)
        glView.setRenderer(render)
        setContentView(glView)


        val photo = application.assets.open("img/photo.jpg")
        val bitmap = BitmapFactory.decodeStream(photo)

        // process image async in RenderThread with GL context
        render.processImage(bitmap)
    }

    override fun onDestroy() {
        super.onDestroy()
        banubaSdk.deinitialize()
    }

    private fun unzipResources(unzipPath: String) {
        val asset = application.assets.open("bnb-resources.zip")
        val zipFile = File("$unzipPath.zip")

        // copy from assets
        val fout = zipFile.outputStream()
        asset.copyTo(fout)
        fout.close()
        asset.close()

        // unzip
        ZipFile(zipFile).use { zip ->
            zip.entries().asSequence().forEach { entry ->
                val file = File(unzipPath, entry.name)
                if (entry.isDirectory) {
                    file.mkdirs()
                } else {
                    zip.getInputStream(entry).use { input ->
                        file.outputStream().use { output ->
                            input.copyTo(output)
                        }
                    }
                }
            }
        }
    }

    private fun copyEffects(resPath: String) {
        val effects = "effects"
        File(resPath, effects).mkdirs()
        val effectsList = application.assets.list(effects)
        for (e in effectsList) {
            File(resPath, "$effects/$e").mkdirs()
            val files = application.assets.list("$effects/$e")
            for (f in files) {
                val ofile = File(resPath, "$effects/$e/$f")
                val ostream = ofile.outputStream()
                val istream = application.assets.open("$effects/$e/$f")
                istream.copyTo(ostream)
                ostream.close()
                istream.close()
            }
        }
    }
}

class EPRenderer(sdk: BanubaSdk, callback: (Bitmap) -> Unit) : GLSurfaceView.Renderer {

    private val banubaSdk = sdk
    private val completion = callback

    private val effectPlayer: Long = banubaSdk.createEffectPlayer()
    private var imageToProcess: Bitmap? = null

    fun processImage(bitmap: Bitmap) {
        imageToProcess = bitmap
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        banubaSdk.surfaceCreated(effectPlayer, 0, 0);
    }

    override fun onSurfaceChanged(gl: GL10, w: Int, h: Int) {
        banubaSdk.surfaceChanged(effectPlayer, w, h);
        gl.glViewport(0, 0, w, h)
    }

    override fun onDrawFrame(gl: GL10) {

        val bitmap = imageToProcess
        if (bitmap != null) {

            banubaSdk.loadEffect(effectPlayer, "effects/Afro")

            val size = bitmap.rowBytes * bitmap.height
            val byteBuffer = ByteBuffer.allocateDirect(size)
            bitmap.copyPixelsToBuffer(byteBuffer)

            val result = banubaSdk.processPhoto(effectPlayer, byteBuffer, bitmap.width, bitmap.height)
            val image = Bitmap.createBitmap(bitmap.width, bitmap.height, Bitmap.Config.ARGB_8888)
            image.copyPixelsFromBuffer(ByteBuffer.wrap(result))

            completion(image)
        }
    }
}
