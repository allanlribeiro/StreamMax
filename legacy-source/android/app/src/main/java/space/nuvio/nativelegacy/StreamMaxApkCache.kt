package space.nuvio.nativelegacy

import java.io.File

// StreamMax stages official OTA updates under filesDir/dados/atualizacao.
// Prune verified old APKs only: never touch unknown/partial downloads or user data.
internal object StreamMaxApkCache {
    fun prune(dir: File, installed: Long, protectedName: String?, version: (File) -> Long?) {
        val apks = dir.listFiles()?.filter { it.isFile && it.name.endsWith(".apk") } ?: return
        val versions = apks.associateWith { apk ->
            try { version(apk) } catch (_: Exception) { null }
        }
        val newest = apks.filter { (versions[it] ?: -1L) > installed }
            .maxWithOrNull(compareBy<File> { versions[it] ?: -1L }.thenBy { it.lastModified() })
        for (apk in apks) {
            if (apk.name == protectedName) continue
            val v = versions[apk] ?: continue // Unknown/partial; might still be downloading.
            if (v <= installed || (newest != null && apk != newest)) apk.delete()
        }
    }
}
