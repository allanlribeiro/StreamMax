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

        val candidate = listOfNotNull(
            stream.behaviorHints?.filename,
            stream.clientResolve?.filename,
            stream.clientResolve?.torrentName,
            stream.clientResolve?.stream?.raw?.filename,
            stream.clientResolve?.stream?.raw?.torrentName,
            stream.title,
            stream.name,
            stream.description
        ).joinToString(" ").uppercase(Locale.ROOT)

        // Only explicit audio-language tokens count. Do not match standalone
        // "PT-BR" next to SUB/SUBTITLES, or generic "MULTI"/"DUAL AUDIO".
        val audioPattern = Regex("""(?:DUBLADO|DUBLAGEM|AUDIO[ ._-]*PT[ ._-]*BR|PT[ ._-]*BR[ ._-]*AUDIO|AUDIO[ ._-]*PORTUGUES|PORTUGUES[ ._-]*AUDIO|BRAZILIAN[ ._-]*AUDIO)""")
        return if (audioPattern.containsMatchIn(candidate)) PortugueseAudio.CONFIRMED
        else PortugueseAudio.UNKNOWN
    }

    private fun isPortugueseLanguage(value: String): Boolean {
        val normalized = value.trim().lowercase(Locale.ROOT).replace('_', '-')
        return normalized in setOf("pt", "pt-br", "por", "pob", "portuguese", "português", "brazilian portuguese")
    }
}
