package space.nuvio.nativelegacy

import android.app.Activity
import android.graphics.BitmapFactory
import android.graphics.Color
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.util.Log
import android.view.ViewGroup
import android.widget.ImageView

/**
 * StreamMax 1.0.11: opening picture approved by the maintainer.
 * No video decoder, audio session, animation, network or MediaPlayer.
 * Only the approved blue/cyan/violet S. No STREAMMAX lettering.
 *
 * A small overlay covers the underlying SDL boot splash and is released as
 * soon as SDL has rendered a few frames, after a short minimum viewing time.
 */
internal class StreamMaxAberturaEstatica(
    private val activity: Activity,
    private val parent: ViewGroup,
    private val frames: () -> Long,
) {
    companion object {
        const val ARTE = "streammax_inicio_s.webp"
        const val MINIMO_MS = 950L
        const val MAXIMO_MS = 4_000L
        private const val CHECAGEM_MS = 120L
        private const val TAG = "StreamMaxAbertura"
    }

    private val handler = Handler(Looper.getMainLooper())
    private var imagem: ImageView? = null
    private var terminou = false
    private var inicio = 0L
    private val verificar = object : Runnable {
        override fun run() {
            if (terminou) return
            val duracao = SystemClock.elapsedRealtime() - inicio
            // Only SDL startup frames are observed. The static asset does not
            // delay the player, catalog requests, or decode another video.
            val pronto = try { frames() >= 3L } catch (_: Throwable) { false }
            if ((duracao >= MINIMO_MS && pronto) || duracao >= MAXIMO_MS) {
                encerrar(if (pronto) "sdl-pronto" else "limite")
            } else {
                handler.postDelayed(this, CHECAGEM_MS)
            }
        }
    }

    fun iniciar() {
        if (terminou || imagem != null) return
        val bitmap = try {
            activity.assets.open(ARTE).use { BitmapFactory.decodeStream(it) }
        } catch (error: Exception) {
            Log.w(TAG, "splash estatico indisponivel: " + error.javaClass.simpleName)
            null
        } ?: return
        val view = ImageView(activity).apply {
            isFocusable = false
            isFocusableInTouchMode = false
            scaleType = ImageView.ScaleType.CENTER_CROP
            setBackgroundColor(Color.BLACK)
            setImageBitmap(bitmap)
            contentDescription = null
        }
        imagem = view
        inicio = SystemClock.elapsedRealtime()
        parent.addView(view, ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT))
        handler.postDelayed(verificar, CHECAGEM_MS)
        Log.i(TAG, "imagem S exibida, sem MP4")
    }

    fun encerrar(motivo: String = "saida") {
        if (terminou) return
        terminou = true
        handler.removeCallbacks(verificar)
        imagem?.let { view ->
            imagem = null
            (view.parent as? ViewGroup)?.removeView(view)
            view.setImageDrawable(null)
        }
        Log.i(TAG, "fim: $motivo")
    }
}
