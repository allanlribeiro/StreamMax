package space.nuvio.nativelegacy

import java.io.File
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test
import org.junit.rules.TemporaryFolder

class StreamMaxApkCacheTest {
    @get:Rule val tmp = TemporaryFolder()

    @Test fun removesInstalledOrSupersededButKeepsUnknownAndUserData() {
        val dir = tmp.newFolder("atualizacao")
        val old = File(dir, "old.apk").apply { writeText("old") }
        val current = File(dir, "current.apk").apply { writeText("current") }
        val next = File(dir, "next.apk").apply { writeText("next") }
        val latest = File(dir, "latest.apk").apply { writeText("latest") }
        val partial = File(dir, "partial.apk").apply { writeText("partial") }
        val durable = File(dir, "preferencias.txt").apply { writeText("keep") }
        val codes = mapOf(old to 20015L, current to 20016L,
            next to 20017L, latest to 20018L)
        StreamMaxApkCache.prune(dir, 20016L, null) { codes[it] }
        assertFalse(old.exists())
        assertFalse(current.exists())
        assertFalse(next.exists())
        assertTrue(latest.exists())
        assertTrue(partial.exists())
        assertTrue(durable.exists())
    }

    @Test fun protectsInProgressUpdateEvenIfOlder() {
        val dir = tmp.newFolder("protected")
        val protected = File(dir, "active.apk").apply { writeText("active") }
        val obsolete = File(dir, "obsolete.apk").apply { writeText("old") }
        StreamMaxApkCache.prune(dir, 20018L, protected.name) { 20016L }
        assertTrue(protected.exists())
        assertFalse(obsolete.exists())
    }

    @Test fun doesNotDeleteUnrecognizedArchivesOnVersionError() {
        val dir = tmp.newFolder("unknown")
        val partial = File(dir, "still-downloading.apk").apply { writeText("incomplete") }
        StreamMaxApkCache.prune(dir, 20018L, null) { throw IllegalArgumentException("partial") }
        assertTrue(partial.exists())
    }
}
