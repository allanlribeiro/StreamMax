package space.nuvio.nativelegacy

import androidx.media3.common.C
import androidx.media3.common.util.UnstableApi
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

/** Policy checks only. Real EGL/GL quality and frame drops require TCL tests. */
@androidx.annotation.OptIn(UnstableApi::class)
class StreamMaxImagePreviewTest {
    @Test fun acceptsSdrHdCandidates() {
        assertNull(StreamMaxImagePreview.refusal(1280, 720, C.COLOR_TRANSFER_SDR, 24f, false))
        assertNull(StreamMaxImagePreview.refusal(1920, 1080, C.COLOR_TRANSFER_SDR, 30f, false))
    }

    @Test fun neverAcceptsHdrOrUhdOrDrm() {
        assertEquals("4k", StreamMaxImagePreview.refusal(3840, 2160, C.COLOR_TRANSFER_SDR, 24f, false))
        assertEquals("hdr", StreamMaxImagePreview.refusal(1920, 1080, C.COLOR_TRANSFER_ST2084, 24f, false))
        assertEquals("hdr", StreamMaxImagePreview.refusal(1920, 1080, C.COLOR_TRANSFER_HLG, 24f, false))
        assertEquals("drm", StreamMaxImagePreview.refusal(1920, 1080, C.COLOR_TRANSFER_SDR, 24f, true))
        assertEquals("high_fps", StreamMaxImagePreview.refusal(1280, 720, C.COLOR_TRANSFER_SDR, 60f, false))
    }
}
