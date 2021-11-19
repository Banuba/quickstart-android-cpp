package com.banuba.sdk.example.quickstart_cpp

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.opengl.GLSurfaceView
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.banuba.sdk.example.common.BANUBA_CLIENT_TOKEN
import com.banuba.sdk.utils.ContextProvider
import kotlinx.android.synthetic.main.activity_main.*
import java.io.*
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
            File(resPath).mkdirs()
            copyAssets(File(resPath), "", listOf("effects"))
        }

        ContextProvider.setContext(applicationContext)

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
        render.onDestroy()
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

        File(unzipPath).mkdirs()

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

    @Throws(IOException::class)
    fun copyAssets(
        baseFolder: File?,
        path: String,
        assetsToCopy: List<String?>
    ) {
        val fileList = application.assets.list(path)
        if (fileList?.isEmpty() == true) {
            val file = File(baseFolder, path)
            val parent = file.parentFile
            if (!parent.exists() && !file.parentFile.mkdirs()) {
                throw IOException(
                    "Failed to create $parent. Check if you have `write` permissions"
                )
            }
            application.assets.open(path).use { inputStream ->
                FileOutputStream(file).use { outputStream ->
                    processStreams(inputStream, outputStream)
                }
            }
        } else {
            for (children in fileList) {
                if (assetsToCopy.contains(children)) {
                    val fullPath = File(path, children).path
                    copyAssets(
                        baseFolder,
                        fullPath,
                        application.assets.list(fullPath).toList()
                    )
                }
            }
        }
    }
    @Throws(IOException::class)
    private fun processStreams(inputStream: InputStream, outputStream: OutputStream) {
        BufferedInputStream(inputStream).use { `in` ->
            BufferedOutputStream(outputStream).use { out ->
                val buffer = ByteArray(10240)
                var bytesRead: Int
                while (`in`.read(buffer).also { bytesRead = it } >= 0) {
                    out.write(buffer, 0, bytesRead)
                }
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

            banubaSdk.loadEffect(effectPlayer, "effects/TrollGrandma")

            // perform 1 draw call to prepare rendering pipeline
            banubaSdk.processPhoto(effectPlayer, ByteBuffer.allocateDirect(4), 1, 1)

            val size = bitmap.rowBytes * bitmap.height
            val byteBuffer = ByteBuffer.allocateDirect(size)
            bitmap.copyPixelsToBuffer(byteBuffer)

            val result = banubaSdk.processPhoto(
                effectPlayer,
                byteBuffer,
                bitmap.width,
                bitmap.height
            )
            val image = Bitmap.createBitmap(bitmap.width, bitmap.height, Bitmap.Config.ARGB_8888)
            image.copyPixelsFromBuffer(ByteBuffer.wrap(result))

            completion(image)
        }
    }

    fun onDestroy() {
        banubaSdk.surfaceDestroyed(effectPlayer)
        banubaSdk.destroyEffectPlayer(effectPlayer)
    }
}
