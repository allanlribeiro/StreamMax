package com.nuvio.tv.core.streams

import com.nuvio.tv.domain.model.Stream
import com.nuvio.tv.domain.model.StreamBehaviorHints
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class StreamSourceTechnicalSummaryTest {
    private fun stream(filename: String?): Stream = Stream(
        name = "Torrentio TB", title = "Filme 4K", description = null,
        url = "https://example.test/video", ytId = null, infoHash = null,
        fileIdx = null, externalUrl = null, addonName = "Torrentio TB",
        addonLogo = null,
        behaviorHints = StreamBehaviorHints(
            notWebReady = null, bingeGroup = null, countryWhitelist = null,
            proxyHeaders = null, videoSize = 25_900_000_000, filename = filename
        )
    )

    @Test fun sourceFileNameAndSizeAreShown() {
        val result = StreamSourceTechnicalSummary.describe(stream("Movie.DUAL.AUDIO.2160p.mkv"))
        assertTrue(result.contains("Arquivo: Movie.DUAL.AUDIO.2160p.mkv"))
        assertTrue(result.contains("25.9 GB"))
    }

    @Test fun dualAudioWithoutPortugueseEvidenceIsNotLabeledPortuguese() {
        val result = StreamSourceTechnicalSummary.describe(stream("Movie.DUAL.AUDIO.2160p.mkv"))
        assertFalse(result.contains("Áudio português indicado"))
    }

    @Test fun dubbedReleaseIsLabeledAsSourceClaim() {
        val result = StreamSourceTechnicalSummary.describe(stream("Movie.DUBLADO.2160p.mkv"))
        assertTrue(result.contains("Áudio português indicado"))
    }
}
