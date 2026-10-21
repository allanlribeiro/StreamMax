package space.nuvio.nativelegacy

import android.app.Activity
import android.graphics.Color
import android.media.MediaPlayer
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.ViewGroup

/**
 * Video de abertura do StreamMax, somente na primeira criacao da Activity.
 *
 * O arquivo opcional assets/streammax_abertura.mp4 deve ser o MP4 aprovado pelo
 * usuario (720p H.264/AAC). Sem arquivo ou sem decoder, o splash SDL existente
 * continua intacto. Usa MediaPlayer separado SOMENTE durante os 4,5 segundos;
 * nao instancia Media3 nem disputa a sessao de reproducao do aplicativo.
 */
internal class StreamMaxAberturaVideo(
    private val activity: Activity,
    private val parent: ViewGroup,
) {
    private val handler = Handler(Looper.getMainLooper())
    private var surface: SurfaceView? = null
    private var player: MediaPlayer? = null
    private var encerrado = false
    private val limite = Runnable { encerrar("prazo") }
    private val tag = "StreamMaxAbertura"

    fun iniciar() {
        if (surface != null || encerrado) return
        // openFd exige asset nao comprimido (MP4 deve ficar em noCompress).
        // Nao mostrar camada preta sem garantir que o arquivo realmente existe.
        try {
            activity.assets.openFd("streammax_abertura.mp4").use { descritor ->
                if (descritor.length <= 0L) return
            }
        } catch (_: Exception) {
            Log.i(tag, "video nao empacotado; splash estatico mantido")
            return
        }

        val video = SurfaceView(activity).apply {
            isFocusable = false
            isFocusableInTouchMode = false
            setBackgroundColor(Color.BLACK)
            setZOrderOnTop(true) // acima do SDL SurfaceView e da camada Media3
        }
        surface = video
        Log.i(tag, "asset presente; aguardando superficie de video")
        video.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                if (encerrado || surface !== video) return
                try {
                    val mp = MediaPlayer()
                    player = mp
                    mp.setOnPreparedListener {
                        Log.i(tag, "video preparado: ${it.videoWidth}x${it.videoHeight}")
                        if (!encerrado) it.start()
                    }
                    mp.setOnInfoListener { _, what, _ ->
                        if (what == MediaPlayer.MEDIA_INFO_VIDEO_RENDERING_START)
                            Log.i(tag, "primeiro quadro do MP4 exibido")
                        false
                    }
                    mp.setOnCompletionListener { encerrar("concluido") }
                    mp.setOnErrorListener { _, what, extra ->
                        Log.w(tag, "reproducao indisponivel: $what/$extra")
                        encerrar("erro")
                        true
                    }
                    activity.assets.openFd("streammax_abertura.mp4").use { arquivo ->
                        mp.setDataSource(arquivo.fileDescriptor, arquivo.startOffset, arquivo.length)
                    }
                    mp.setDisplay(holder)
                    mp.isLooping = false
                    mp.prepareAsync()
                } catch (e: Exception) {
                    Log.w(tag, "fallback estatico: ${e.javaClass.simpleName}")
                    encerrar("falha-preparo")
                }
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}
            override fun surfaceDestroyed(holder: SurfaceHolder) { encerrar("superficie") }
        })
        parent.addView(video, ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        ))
        // Nao segurar controle nem a Home no caso de decoder que nunca responde.
        handler.postDelayed(limite, 6_500L)
    }

    fun ativa(): Boolean = surface != null && !encerrado

    fun encerrar(motivo: String = "saida") {
        if (encerrado) return
        encerrado = true
        handler.removeCallbacks(limite)
        player?.let {
            try { it.setOnCompletionListener(null); it.setOnErrorListener(null); it.reset() } catch (_: Exception) {}
            try { it.release() } catch (_: Exception) {}
        }
        player = null
        surface?.let { v ->
            surface = null
            (v.parent as? ViewGroup)?.removeView(v)
        }
        Log.i(tag, "fim: $motivo")
    }
}
