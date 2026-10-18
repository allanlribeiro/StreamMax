package com.nuvio.tv.core.streams

import com.nuvio.tv.domain.model.Stream
import java.util.Locale

/** Source facts supplied by the addon/debrid resolver; does not inspect actual playback tracks. */
internal object StreamSourceTechnicalSummary {
    fun describe(stream: Stream): String {
        val raw = stream.clientResolve?.stream?.raw
        val parsed = raw?.parsed
        val facts = linkedSetOf<String>()
        fun include(value: String?) {
            value?.trim()?.takeIf(String::isNotEmpty)?.let(facts::add)
        }
        include(parsed?.resolution)
        include(parsed?.quality)
        include(parsed?.codec)
        parsed?.hdr.orEmpty().forEach(::include)
        parsed?.audio.orEmpty().forEach(::include)
        parsed?.channels.orEmpty().forEach(::include)
        parsed?.languages.orEmpty().forEach(::include)
        if (StreamAudioLanguageDetector.portugueseAudio(stream) ==
            StreamAudioLanguageDetector.PortugueseAudio.CONFIRMED
        ) include("Áudio português indicado")
        val sizeBytes = raw?.size ?: stream.behaviorHints?.videoSize
        if (sizeBytes != null && sizeBytes > 0) {
            include(String.format(Locale.ROOT, "%.1f GB", sizeBytes / 1_000_000_000.0))
        }
        // An unparsed release name can contain DUAL/MULTI tags omitted by parsed metadata.
        // Present it as a filename, not as a guaranteed playback-audio classification.
        val filename = stream.behaviorHints?.filename
            ?: stream.clientResolve?.filename
            ?: raw?.filename
            ?: stream.clientResolve?.torrentName
            ?: raw?.torrentName
        filename?.takeIf(String::isNotBlank)?.let { include("Arquivo: $it") }
        return facts.joinToString(" · ")
    }
}
