package space.nuvio.nativelegacy

import android.content.Context
import android.content.pm.ApplicationInfo
import android.opengl.GLES20
import android.provider.Settings
import androidx.media3.common.C
import androidx.media3.common.VideoFrameProcessingException
import androidx.media3.common.util.GlProgram
import androidx.media3.common.util.GlUtil
import androidx.media3.common.util.Size
import androidx.media3.common.util.UnstableApi
import androidx.media3.effect.BaseGlShaderProgram
import androidx.media3.effect.GlEffect
import androidx.media3.effect.GlShaderProgram
import androidx.media3.effect.PassthroughShaderProgram

/**
 * 1.0.11 proof-of-concept only; NEVER enabled in a production build.
 *
 * Debug-only switch: adb shell settings put global streammax_image_preview 1
 * Undo: adb shell settings delete global streammax_image_preview
 * Not AI super-resolution. Single-pass edge-adaptive, very mild 5-tap unsharp mask
 * at the source resolution. Real 720p/1080p performance MUST be measured on TCL P7K.
 */
internal object StreamMaxImagePreview {
    const val SETTING = "streammax_image_preview"

    fun requested(context: Context): Boolean {
        if (context.applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE == 0) return false
        return try { Settings.Global.getInt(context.contentResolver, SETTING, 0) == 1 }
        catch (_: Exception) { false }
    }

    /** null means that this candidate has not been ruled out by metadata yet. */
    fun refusal(width: Int, height: Int, transfer: Int, fps: Float, drm: Boolean): String? {
        if (drm) return "drm"
        if (width > 1920 || height > 1080) return "4k"
        if (transfer == C.COLOR_TRANSFER_ST2084 || transfer == C.COLOR_TRANSFER_HLG) return "hdr"
        if (fps > 30.5f) return "high_fps"
        return null
    }

    fun effect(): List<androidx.media3.common.Effect> = listOf(MildSharpenEffect(0.22f))
}

@UnstableApi
private class MildSharpenEffect(private val strength: Float) : GlEffect {
    override fun toGlShaderProgram(context: Context, useHdr: Boolean): GlShaderProgram =
        if (useHdr) PassthroughShaderProgram() else MildSharpenShader(strength)
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
            precision mediump float;
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
