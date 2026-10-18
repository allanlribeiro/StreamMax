package com.nuvio.tv.core.streams

import com.nuvio.tv.domain.model.Stream
import java.util.Locale

/**
 * Conservative language classification for source listings.
 *
 * Torrentio metadata is often incomplete: "DUAL AUDIO" alone does not prove
 * Portuguese is one of the tracks. Never infer language from a subtitle tag.
 */
internal object StreamAudioLanguageDetector {
    enum class PortugueseAudio { CONFIRMED, UNKNOWN }

    fun portugueseAudio(stream: Stream): PortugueseAudio {
        val declaredLanguages = stream.clientResolve?.stream?.raw?.parsed?.languages.orEmpty()
        if (declaredLanguages.any(::isPortugueseLanguage)) return PortugueseAudio.CONFIRMED

        val fields = listOfNotNull(
            stream.behaviorHints?.filename,
            stream.clientResolve?.filename,
            stream.clientResolve?.torrentName,
            stream.clientResolve?.stream?.raw?.filename,
            stream.clientResolve?.stream?.raw?.torrentName,
            stream.title,
            stream.name,
            stream.description
        )
        return if (fields.any(::hasExplicitPortugueseAudio)) PortugueseAudio.CONFIRMED
        else PortugueseAudio.UNKNOWN
    }

    private fun hasExplicitPortugueseAudio(value: String): Boolean {
        val text = value.uppercase(Locale.ROOT)
        // Check token boundaries: "UNDUBLADO" must not be taken as "DUBLADO".
        val explicitAudio = Regex(
            """(?:^|[^A-Z0-9])(?:DUBLADO|DUBLAGEM|ÁUDIO[ ._-]*PT[ ._-]*BR|AUDIO[ ._-]*PT[ ._-]*BR|PT[ ._-]*BR[ ._-]*ÁUDIO|PT[ ._-]*BR[ ._-]*AUDIO|ÁUDIO[ ._-]*PORTUGU[EÊ]S|AUDIO[ ._-]*PORTUGU[EÊ]S|PORTUGU[EÊ]S[ ._-]*ÁUDIO|PORTUGU[EÊ]S[ ._-]*AUDIO|BRAZILIAN[ ._-]*AUDIO)(?:$|[^A-Z0-9])"""
        )
        return explicitAudio.containsMatchIn(text)
    }

    private fun isPortugueseLanguage(value: String): Boolean {
        val normalized = value.trim().lowercase(Locale.ROOT).replace('_', '-')
        return normalized in setOf("pt", "pt-br", "por", "pob", "portuguese", "português", "brazilian portuguese")
    }
}
