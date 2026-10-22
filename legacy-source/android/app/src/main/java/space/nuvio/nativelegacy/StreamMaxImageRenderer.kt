package space.nuvio.nativelegacy

import android.media.MediaFormat
import androidx.media3.common.C
import androidx.media3.common.Format
import androidx.media3.common.PlaybackException
import androidx.media3.common.util.UnstableApi
import androidx.media3.exoplayer.DecoderReuseEvaluation
import androidx.media3.exoplayer.FormatHolder
import androidx.media3.exoplayer.ForwardingRenderer
import androidx.media3.exoplayer.RendererConfiguration
import androidx.media3.exoplayer.source.MediaSource
import androidx.media3.exoplayer.source.SampleStream
import androidx.media3.exoplayer.video.MediaCodecVideoRenderer

/** Marker caught by the application to rebuild an entirely native Surface pipeline. */
internal class StreamMaxImageBypass : IllegalStateException("Native video pipeline required")

/** The wrapper sees ALL selectable formats before Media3's final enable method runs.
 * Never send even an empty setVideoEffects message to the ordinary/protected path:
 * in Media3 1.8 that non-null effects list is enough to allocate a GL VideoSink.
 */
@UnstableApi
internal class StreamMaxImageRenderer(
    private val video: GuardedImageVideoRenderer,
    private val session: StreamMaxImageSession,
    private val displayWidth: Int,
    private val displayHeight: Int
) : ForwardingRenderer(video) {
    override fun enable(configuration: RendererConfiguration, formats: Array<Format>, stream: SampleStream,
                        positionUs: Long, joining: Boolean, mayRenderStartOfStream: Boolean,
                        startPositionUs: Long, offsetUs: Long, mediaPeriodId: MediaSource.MediaPeriodId) {
        val eligible = StreamMaxImagePreview.shouldActivate(formats, session.requested && !session.disabled,
            displayWidth, displayHeight, configuration.tunneling)
        if (session.active && !eligible) throw video.bypass(formats.firstOrNull())
        if (eligible) {
            video.setVideoEffects(StreamMaxImagePreview.effect())
            session.active = true
            android.util.Log.i("NvPlayer", "[imagem] HD SDR enhancement active; source resolution retained")
        }
        super.enable(configuration, formats, stream, positionUs, joining, mayRenderStartOfStream,
            startPositionUs, offsetUs, mediaPeriodId)
    }
}

@UnstableApi
internal class GuardedImageVideoRenderer(builder: MediaCodecVideoRenderer.Builder,
                                         private val session: StreamMaxImageSession) : MediaCodecVideoRenderer(builder) {
    fun bypass(format: Format?) = createRendererException(StreamMaxImageBypass(), format,
        PlaybackException.ERROR_CODE_VIDEO_FRAME_PROCESSOR_INIT_FAILED)

    private fun guard(format: Format) {
        if (session.active && (session.disabled || StreamMaxImagePreview.refusal(format) != null)) throw bypass(format)
    }

    override fun onStreamChanged(formats: Array<Format>, startPositionUs: Long, offsetUs: Long,
                                 mediaPeriodId: MediaSource.MediaPeriodId) {
        formats.forEach(::guard)
        super.onStreamChanged(formats, startPositionUs, offsetUs, mediaPeriodId)
    }

    override fun onInputFormatChanged(formatHolder: FormatHolder): DecoderReuseEvaluation? {
        formatHolder.format?.let(::guard)
        return super.onInputFormatChanged(formatHolder)
    }

    override fun maybeInitializeProcessingPipeline(format: Format): Boolean {
        // Last check before GL initialization, including in-band format updates.
        guard(format)
        return super.maybeInitializeProcessingPipeline(format)
    }

    override fun onOutputFormatChanged(format: Format, mediaFormat: MediaFormat?) {
        guard(format)
        if (session.active && mediaFormat != null) {
            val width = if (mediaFormat.containsKey(MediaFormat.KEY_WIDTH)) mediaFormat.getInteger(MediaFormat.KEY_WIDTH) else format.width
            val height = if (mediaFormat.containsKey(MediaFormat.KEY_HEIGHT)) mediaFormat.getInteger(MediaFormat.KEY_HEIGHT) else format.height
            val transfer = if (mediaFormat.containsKey(MediaFormat.KEY_COLOR_TRANSFER)) mediaFormat.getInteger(MediaFormat.KEY_COLOR_TRANSFER) else C.COLOR_TRANSFER_SDR
            if (width > 1920 || height > 1088 || transfer != C.COLOR_TRANSFER_SDR) throw bypass(format)
        }
        super.onOutputFormatChanged(format, mediaFormat)
    }
}
