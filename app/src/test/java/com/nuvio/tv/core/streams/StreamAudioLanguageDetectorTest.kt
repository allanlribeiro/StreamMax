package com.nuvio.tv.core.streams

import com.nuvio.tv.domain.model.Stream
import com.nuvio.tv.domain.model.StreamClientResolve
import com.nuvio.tv.domain.model.StreamClientResolveParsed
import com.nuvio.tv.domain.model.StreamClientResolveRaw
import com.nuvio.tv.domain.model.StreamClientResolveStream
import org.junit.Assert.assertEquals
import org.junit.Test

class StreamAudioLanguageDetectorTest {
    private fun stream(
        title: String = "",
        languages: List<String>? = null
    ) = Stream(
        name = null,
        title = title,
        description = null,
        url = "https://example.test/video",
        ytId = null,
        infoHash = null,
        fileIdx = null,
        externalUrl = null,
        behaviorHints = null,
        addonName = "Torrentio TB",
        addonLogo = null,
        clientResolve = languages?.let {
            StreamClientResolve(
                type = null, infoHash = null, fileIdx = null, magnetUri = null,
                sources = null, torrentName = null, filename = null,
                mediaType = null, mediaId = null, mediaOnlyId = null, title = null,
                season = null, episode = null, service = null,
                serviceIndex = null, serviceExtension = null, isCached = null,
                stream = StreamClientResolveStream(
                    StreamClientResolveRaw(
                        torrentName = null, filename = null, size = null,
                        folderSize = null, tracker = null, indexer = null, network = null,
                        parsed = StreamClientResolveParsed(
                            rawTitle = null, parsedTitle = null, year = null,
                            resolution = null, seasons = null, episodes = null,
                            quality = null, hdr = null, codec = null, audio = null,
                            channels = null, languages = it, group = null, network = null,
                            edition = null, duration = null, bitDepth = null,
                            extended = null, theatrical = null, remastered = null,
                            unrated = null
                        )
                    )
                )
            )
        }
    )

    @Test fun explicitPortugueseAudio() {
        assertEquals(StreamAudioLanguageDetector.PortugueseAudio.CONFIRMED,
            StreamAudioLanguageDetector.portugueseAudio(stream("Filme.DUBLADO.2160p")))
    }

    @Test fun structuredMetadataPortuguese() {
        assertEquals(StreamAudioLanguageDetector.PortugueseAudio.CONFIRMED,
            StreamAudioLanguageDetector.portugueseAudio(stream("Movie 4K", listOf("pt-BR"))))
    }

    @Test fun genericDualAudioIsNotAutomaticallyPortuguese() {
        assertEquals(StreamAudioLanguageDetector.PortugueseAudio.UNKNOWN,
            StreamAudioLanguageDetector.portugueseAudio(stream("Movie.DUAL.AUDIO.4K")))
    }

    @Test fun subtitleLanguageIsNotAudioLanguage() {
        assertEquals(StreamAudioLanguageDetector.PortugueseAudio.UNKNOWN,
            StreamAudioLanguageDetector.portugueseAudio(stream("Movie.SUB.PT-BR.4K")))
    }
}
