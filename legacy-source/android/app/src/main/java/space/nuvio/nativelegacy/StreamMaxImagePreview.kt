package space.nuvio.nativelegacy

import android.content.Context
import android.opengl.GLES20
import androidx.media3.common.C
import androidx.media3.common.VideoFrameProcessingException
import androidx.media3.common.util.GlProgram
import androidx.media3.common.util.GlUtil
import androidx.media3.common.util.Size
import androidx.media3.common.util.UnstableApi
import androidx.media3.effect.BaseGlShaderProgram
import androidx.media3.effect.GlEffect
import androidx.media3.effect.GlShaderProgram

/** Lightweight source-resolution sharpening to assist the display's native upscaler.
 * No AI, no invented detail, no 4K intermediate textures. Disabled by default.
 * Format admission happens on the playback thread BEFORE renderer.enable creates a GL sink.
 */
internal object StreamMaxImagePreview {
    fun refusal(format: androidx.media3.common.Format): String? = refusal(
        format.width, format.height, format.colorInfo?.colorTransfer ?: C.INDEX_UNSET,
        format.frameRate, format.drmInitData != null, format.sampleMimeType, format.codecs,
        format.colorInfo?.colorSpace ?: C.INDEX_UNSET, format.rotationDegrees,
        format.pixelWidthHeightRatio
    )

    fun refusal(width: Int, height: Int, transfer: Int, fps: Float, drm: Boolean,
                mime: String? = null, codecs: String? = null,
                colorSpace: Int = C.INDEX_UNSET, rotation: Int = 0, pixelRatio: Float = 1f): String? {
        if (drm) return "drm"
        val codec = codecs.orEmpty().lowercase(java.util.Locale.ROOT)
        if (mime == "video/dolby-vision" || listOf("dvhe", "dvh1", "dvav", "dva1").any { codec.contains(it) }) return "dolby_vision"
        if (width > 1920 || height > 1080) return "4k_or_above_hd"
        if (transfer != C.COLOR_TRANSFER_SDR || colorSpace == C.COLOR_SPACE_BT2020) return "hdr_or_unknown_color"
        // Unknown metadata is NEVER permission to process; cropped widescreen HD is supported.
        if (width !in 1280..1920 || height !in 480..1080) return "unknown_or_non_hd_size"
        if (!fps.isFinite() || fps <= 0f || fps > 30.5f) return "unknown_or_high_fps"
        if (rotation != 0 || pixelRatio != 1f) return "rotated_or_anamorphic"
        if (mime !in setOf("video/avc", "video/hevc", "video/x-vnd.on2.vp9", "video/av01")) return "unknown_codec"
        return null
    }

    fun shouldActivate(formats: Array<androidx.media3.common.Format>, requested: Boolean,
                       displayWidth: Int, displayHeight: Int, tunneling: Boolean): Boolean =
        requested && !tunneling && formats.isNotEmpty() && formats.all {
            refusal(it) == null && displayWidth > it.width && displayHeight > it.height
        }

    fun thermalLimited(context: Context): Boolean = try {
        val power = context.getSystemService(Context.POWER_SERVICE) as? android.os.PowerManager
        power?.isPowerSaveMode == true || (android.os.Build.VERSION.SDK_INT >= 29 &&
            power != null && power.currentThermalStatus >= android.os.PowerManager.THERMAL_STATUS_MODERATE)
    } catch (_: Exception) { true }

    fun effect(): List<androidx.media3.common.Effect> = listOf(MildSharpenEffect(0.22f))
}

/** Session-local latch: no repeated automatic retries after a performance or GL failure. */
internal class StreamMaxImageSession(val requested: Boolean) {
    @Volatile var active = false
    @Volatile var disabled = false
    private var previousDrops: Long? = null
    private var previousTime = 0L
    private var badWindows = 0

    fun sample(nowMs: Long, totalDrops: Long, playing: Boolean, thermalLimited: Boolean): Boolean {
        if (!active || disabled) return false
        if (thermalLimited) return true
        val previous = previousDrops
        val elapsed = nowMs - previousTime
        previousDrops = totalDrops
        previousTime = nowMs
        if (!playing || previous == null || totalDrops < previous || elapsed !in 750..2500) {
            badWindows = 0
            return false
        }
        val drops = totalDrops - previous
        badWindows = if (drops * 1000 >= 4 * elapsed) badWindows + 1 else 0
        return badWindows >= 2
    }
}

@UnstableApi
private class MildSharpenEffect(private val strength: Float) : GlEffect {
    override fun toGlShaderProgram(context: Context, useHdr: Boolean): GlShaderProgram =
        if (useHdr) throw VideoFrameProcessingException(IllegalStateException("HDR excluded from image enhancement"))
        else MildSharpenShader(strength)
}

@UnstableApi
private class MildSharpenShader(private val strength: Float) :
    BaseGlShaderProgram(/* useHighPrecisionColorComponents= */ false, /* texturePoolCapacity= */ 1) {

    private val program = try {
        GlProgram(
            """
            attribute vec4 aFramePosition;
            varying vec2 vUv;
            void main() {
                gl_Position = aFramePosition;
                vUv = 0.5 * (aFramePosition.xy + 1.0);
            }
            """.trimIndent(),
            """
            precision highp float;  // 1080p texel offset must not quantize to zero.
            uniform sampler2D uTexSampler;
            uniform vec2 uPixel;
            uniform float uStrength;
            varying vec2 vUv;
            void main() {
                vec4 center = texture2D(uTexSampler, vUv);
                vec3 sides =
                    texture2D(uTexSampler, vUv + vec2(uPixel.x, 0.0)).rgb +
                    texture2D(uTexSampler, vUv - vec2(uPixel.x, 0.0)).rgb +
                    texture2D(uTexSampler, vUv + vec2(0.0, uPixel.y)).rgb +
                    texture2D(uTexSampler, vUv - vec2(0.0, uPixel.y)).rgb;
                vec3 detail = center.rgb - sides * 0.25;
                float contrast = abs(dot(detail, vec3(0.2126, 0.7152, 0.0722)));
                float edge = smoothstep(0.018, 0.080, contrast);
                gl_FragColor = vec4(clamp(center.rgb + uStrength * edge * detail, 0.0, 1.0), center.a);
            }
            """.trimIndent()
        )
    } catch (error: GlUtil.GlException) {
        throw VideoFrameProcessingException(error)
    }

    init {
        program.setBufferAttribute(
            "aFramePosition", GlUtil.getNormalizedCoordinateBounds(),
            GlUtil.HOMOGENEOUS_COORDINATE_VECTOR_SIZE
        )
        program.setFloatUniform("uStrength", strength)
    }

    override fun configure(inputWidth: Int, inputHeight: Int): Size {
        program.setFloatsUniform("uPixel", floatArrayOf(1f / inputWidth, 1f / inputHeight))
        return Size(inputWidth, inputHeight)  // no 4K intermediate or costly rescale
    }

    override fun drawFrame(inputTexId: Int, presentationTimeUs: Long) {
        try {
            program.use()
            program.setSamplerTexIdUniform("uTexSampler", inputTexId, 0)
            program.bindAttributesAndUniforms()
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
        } catch (error: GlUtil.GlException) {
            throw VideoFrameProcessingException(error, presentationTimeUs)
        }
    }

    override fun release() {
        try { program.delete() }
        finally { super.release() }
    }
}
