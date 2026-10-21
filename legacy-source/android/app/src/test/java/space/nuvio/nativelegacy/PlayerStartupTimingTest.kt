package space.nuvio.nativelegacy

import org.junit.Assert.*
import org.junit.Test

class PlayerStartupTimingTest {
    @Test fun completeStartupRecordsFiveDurations() {
        var now = 100L
        val timing = PlayerStartupTiming { now }
        timing.mark(PlayerStartupTiming.Phase.SELECTED)
        now = 120
        timing.mark(PlayerStartupTiming.Phase.RESOLVED)
        now = 155
        timing.mark(PlayerStartupTiming.Phase.FIRST_BYTE)
        now = 180
        timing.mark(PlayerStartupTiming.Phase.PREPARED)
        now = 200
        timing.mark(PlayerStartupTiming.Phase.FIRST_FRAME)
        val result = timing.finish(PlayerStartupTiming.Outcome.SUCCESS)!!
        assertEquals(20L, result.sourceResolveMs)
        assertEquals(35L, result.firstByteMs)
        assertEquals(25L, result.prepareMs)
        assertEquals(20L, result.firstFrameMs)
        assertEquals(100L, result.startupTotalMs)
        assertEquals(PlayerStartupTiming.Outcome.SUCCESS, result.outcome)
    }

    @Test fun missingPhasesRemainUnknownOnError() {
        val timing = PlayerStartupTiming { 10L }
        timing.mark(PlayerStartupTiming.Phase.SELECTED)
        val result = timing.finish(PlayerStartupTiming.Outcome.ERROR)!!
        assertNull(result.firstByteMs)
        assertNull(result.startupTotalMs)
        assertEquals(PlayerStartupTiming.Outcome.ERROR, result.outcome)
    }

    @Test fun cancellationIsTerminalAndDuplicateMarksAreIgnored() {
        var now = 1L
        val timing = PlayerStartupTiming { now }
        timing.mark(PlayerStartupTiming.Phase.SELECTED)
        now = 5L
        timing.mark(PlayerStartupTiming.Phase.SELECTED)
        now = 9L
        timing.mark(PlayerStartupTiming.Phase.RESOLVED)
        val result = timing.finish(PlayerStartupTiming.Outcome.CANCELLED)!!
        assertEquals(8L, result.sourceResolveMs)
        assertNull(timing.finish(PlayerStartupTiming.Outcome.SUCCESS))
        timing.mark(PlayerStartupTiming.Phase.FIRST_FRAME)
    }
}
