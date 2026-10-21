package space.nuvio.nativelegacy

import android.os.SystemClock

/**
 * Coletor leve de tempos de abertura. Não altera o pipeline de reprodução.
 * Os pontos de medição serão conectados ao fluxo real em commit separado.
 * Nunca armazena URL, token, título ou identificador da fonte.
 */
internal class PlayerStartupTiming(
    private val clockMs: () -> Long = { SystemClock.elapsedRealtime() }
) {
    enum class Phase { SELECTED, RESOLVED, FIRST_BYTE, PREPARED, FIRST_FRAME }
    enum class Outcome { SUCCESS, ERROR, CANCELLED }

    private val times = LongArray(Phase.entries.size) { -1L }

    @Synchronized
    fun mark(phase: Phase) {
        if (finished || times[phase.ordinal] >= 0) return
        times[phase.ordinal] = clockMs()
    }

    private var finished = false

    data class Result(
        val sourceResolveMs: Long?,
        val firstByteMs: Long?,
        val prepareMs: Long?,
        val firstFrameMs: Long?,
        val startupTotalMs: Long?,
        val outcome: Outcome
    )

    @Synchronized
    fun finish(outcome: Outcome): Result? {
        if (finished) return null
        finished = true
        fun delta(a: Phase, b: Phase): Long? {
            val start = times[a.ordinal]
            val end = times[b.ordinal]
            return if (start >= 0 && end >= start) end - start else null
        }
        return Result(
            delta(Phase.SELECTED, Phase.RESOLVED),
            delta(Phase.RESOLVED, Phase.FIRST_BYTE),
            delta(Phase.FIRST_BYTE, Phase.PREPARED),
            delta(Phase.PREPARED, Phase.FIRST_FRAME),
            delta(Phase.SELECTED, Phase.FIRST_FRAME),
            outcome
        )
    }
}
