package space.nuvio.nativelegacy

import androidx.media3.common.C
import androidx.media3.common.ColorInfo
import androidx.media3.common.Format
import androidx.media3.common.util.UnstableApi
import org.junit.Assert.*
import org.junit.Test

/** Admission/fallback policy tests; visual quality and EGL performance require a real TV. */
@androidx.annotation.OptIn(UnstableApi::class)
class StreamMaxImagePreviewTest {
    private fun hd(width: Int = 1280, height: Int = 720, transfer: Int = C.COLOR_TRANSFER_SDR,
                   mime: String = "video/avc", codecs: String = "avc1.640028", fps: Float = 24f): Format =
        Format.Builder().setWidth(width).setHeight(height).setSampleMimeType(mime).setCodecs(codecs)
            .setFrameRate(fps).setColorInfo(ColorInfo.Builder().setColorSpace(C.COLOR_SPACE_BT709)
                .setColorTransfer(transfer).setColorRange(C.COLOR_RANGE_LIMITED).build()).build()

    @Test fun onlyConfirmedHdOnLargerDisplaysIsEligible() {
        assertTrue(StreamMaxImagePreview.shouldActivate(arrayOf(hd()), true, 1920, 1080, false))
        assertTrue(StreamMaxImagePreview.shouldActivate(arrayOf(hd(1920, 1080)), true, 3840, 2160, false))
        assertFalse(StreamMaxImagePreview.shouldActivate(arrayOf(hd(1920, 1080)), true, 1920, 1080, false))
        assertFalse(StreamMaxImagePreview.shouldActivate(arrayOf(hd()), false, 3840, 2160, false))
        assertFalse(StreamMaxImagePreview.shouldActivate(arrayOf(hd()), true, 3840, 2160, true))
        assertFalse(StreamMaxImagePreview.shouldActivate(emptyArray(), true, 3840, 2160, false))
    }

    @Test fun protectedAndMixedAdaptiveTracksNeverActivate() {
        val protected = listOf(hd(3840, 1600), hd(4096, 2160), hd(7680, 4320),
            hd(transfer = C.COLOR_TRANSFER_ST2084), hd(transfer = C.COLOR_TRANSFER_HLG),
            hd(mime = "video/dolby-vision", codecs = "dvhe.08.06"),
            hd(mime = "video/hevc", codecs = "dvh1.05.06"), hd(codecs = "dvav.09.06"),
            hd(codecs = "dva1.09.06"))
        for (format in protected) {
            assertNotNull(StreamMaxImagePreview.refusal(format))
            assertFalse(StreamMaxImagePreview.shouldActivate(arrayOf(hd(), format), true, 3840, 2160, false))
        }
        assertEquals("drm", StreamMaxImagePreview.refusal(1280, 720, C.COLOR_TRANSFER_SDR, 24f, true))
    }

    @Test fun incompleteMetadataAndNonHdStayNative() {
        for (format in listOf(hd().buildUpon().setColorInfo(null).build(), hd(fps = -1f), hd(fps = Float.NaN),
            hd(fps = 60f), hd(640, 480), hd(-1, -1), hd().buildUpon().setRotationDegrees(90).build(),
            hd().buildUpon().setPixelWidthHeightRatio(1.33f).build(),
            hd().buildUpon().setColorInfo(ColorInfo.Builder().setColorSpace(C.COLOR_SPACE_BT2020)
                .setColorTransfer(C.COLOR_TRANSFER_SDR).build()).build())) {
            assertNotNull(StreamMaxImagePreview.refusal(format))
        }
    }

    @Test fun sustainedDropsTriggerNativeRebuildAndThermalPressureIsImmediate() {
        val session = StreamMaxImageSession(true).apply { active = true }
        assertFalse(session.sample(1000, 0, true, false))
        assertFalse(session.sample(2000, 4, true, false))
        assertTrue(session.sample(3000, 8, true, false))
        session.disabled = true
        assertFalse(session.sample(4000, 12, true, true))
        val hot = StreamMaxImageSession(true).apply { active = true }
        assertTrue(hot.sample(1000, 0, true, true))
    }

    @Test fun pausedSeekCounterResetOrDelayedSamplingDoesNotAccumulateFalseAlarms() {
        val session = StreamMaxImageSession(true).apply { active = true }
        assertFalse(session.sample(1000, 0, true, false))
        assertFalse(session.sample(2000, 10, true, false))
        assertFalse(session.sample(3000, 20, false, false))
        assertFalse(session.sample(4000, 21, true, false))
        assertFalse(session.sample(8000, 50, true, false))
        assertFalse(session.sample(9000, 0, true, false))
        assertFalse(session.sample(10000, 0, true, false))
    }
}
