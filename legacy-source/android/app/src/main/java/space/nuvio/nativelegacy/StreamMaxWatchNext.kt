package space.nuvio.nativelegacy

import android.content.BroadcastReceiver
import android.content.ContentUris
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.tvprovider.media.tv.TvContractCompat
import androidx.tvprovider.media.tv.WatchNextProgram
import org.json.JSONObject
import java.security.MessageDigest
import java.util.UUID
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicLong

/** Device-local TV-provider integration. Never sends stream URLs or account credentials. */
object StreamMaxWatchNext {
    const val EXTRA_SCOPE = "com.streammax.tv.WATCH_NEXT_SCOPE"
    private const val PREFIX = "streammax_watchnext_v1:"
    private val worker = Executors.newSingleThreadExecutor { r -> Thread(r, "streammax-watch-next") }
    private val revision = AtomicLong()
    private val main = Handler(Looper.getMainLooper())
    private val fingerprints = HashMap<String, String>() // worker thread only
    private var lastScope = ""
    private data class Row(val id: Long, val key: String, val visible: Boolean, val position: Long)

    private fun supported(c: Context) = Build.VERSION.SDK_INT >= 26 &&
        c.packageManager.hasSystemFeature(PackageManager.FEATURE_LEANBACK)
    private fun prefs(c: Context) = c.getSharedPreferences("streammax_watch_next", Context.MODE_PRIVATE)

    private fun scope(c: Context, owner: String, profile: Int): String {
        val p = prefs(c)
        val salt = p.getString("salt", null) ?: UUID.randomUUID().toString().also { p.edit().putString("salt", it).apply() }
        return MessageDigest.getInstance("SHA-256").digest("$salt|$owner|$profile".toByteArray(Charsets.UTF_8))
            .joinToString("") { "%02x".format(it.toInt() and 255) }
    }

    fun sync(context: Context, payload: ByteArray, onScope: (String, Long) -> Unit) {
        val c = context.applicationContext
        if (!supported(c) || payload.size > 128 * 1024) return
        val ticket = revision.incrementAndGet()
        worker.execute {
            if (revision.get() != ticket) return@execute
            var resolvedScope: String? = null
            var resolvedSession = 0L
            try {
                val json = JSONObject(payload.toString(Charsets.UTF_8))
                val active = if (json.optBoolean("ready")) scope(c, json.optString("owner"), json.optInt("profile", 1)) else ""
                resolvedScope = active
                resolvedSession = json.optLong("session")
                if (active != lastScope) { fingerprints.clear(); lastScope = active }
                val rows = rows(c).associateBy { it.key }.toMutableMap()
                val keep = HashSet<String>()
                val array = json.optJSONArray("items")
                if (active.isNotEmpty() && array != null) for (i in 0 until minOf(array.length(), 12)) {
                    if (revision.get() != ticket) return@execute
                    val item = array.optJSONObject(i) ?: continue
                    val series = item.optBoolean("series")
                    val pos = item.optLong("position", -1)
                    val duration = item.optLong("duration", -1)
                    if (!WatchNextPolicy.unfinished(series, pos, duration, json.optInt("completed", 95))) continue
                    val id = item.optString("id")
                    val season = item.optInt("season")
                    val episode = item.optInt("episode")
                    val link = WatchNextPolicy.playUri(id, series, season, episode)
                    if (link.isEmpty() || item.optString("title").isBlank()) continue
                    val key = "$PREFIX$active:${if (series) "series" else "movie"}:$id:$season:$episode"
                    keep.add(key)
                    val row = rows[key]
                    val p = prefs(c)
                    if (row != null && !row.visible && p.getLong("dismiss_$key", 0) == 0L) {
                        p.edit().putLong("dismiss_$key", System.currentTimeMillis()).putLong("dismissPos_$key", row.position).apply()
                    }
                    val dismissed = p.getLong("dismiss_$key", 0)
                    val engaged = item.optLong("engaged")
                    if (engaged <= 0) continue
                    if (dismissed > 0 && (engaged <= dismissed || kotlin.math.abs(pos - p.getLong("dismissPos_$key", pos)) < 5000)) continue
                    if (revision.get() != ticket) return@execute
                    if (dismissed > 0) p.edit().remove("dismiss_$key").remove("dismissPos_$key").apply()
                    val intent = Intent(Intent.ACTION_VIEW, Uri.parse(link), c, NuvioActivity::class.java)
                        .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
                        .putExtra(EXTRA_SCOPE, active)
                    val publicArt = WatchNextPolicy.publicArtwork(item.optString("art"))
                    val art = if (publicArt.isNotEmpty()) Uri.parse(publicArt) else
                        Uri.parse("android.resource://${c.packageName}/${R.drawable.streammax_banner}")
                    val builder = WatchNextProgram.Builder()
                        .setType(if (series) TvContractCompat.PreviewPrograms.TYPE_TV_EPISODE else TvContractCompat.PreviewPrograms.TYPE_MOVIE)
                        .setWatchNextType(TvContractCompat.WatchNextPrograms.WATCH_NEXT_TYPE_CONTINUE)
                        .setInternalProviderId(key).setContentId(id).setTitle(item.optString("title").take(160))
                        .setPosterArtUri(art).setPosterArtAspectRatio(TvContractCompat.PreviewPrograms.ASPECT_RATIO_16_9)
                        .setIntent(intent).setLastEngagementTimeUtcMillis(engaged)
                        .setLastPlaybackPositionMillis(pos.toInt()).setDurationMillis(duration.toInt())
                    if (series) {
                        builder.setSeasonNumber(season.coerceAtLeast(0).toString(), season.coerceAtLeast(0))
                            .setEpisodeNumber(episode.coerceAtLeast(0).toString(), episode.coerceAtLeast(0))
                        if (item.optString("episodeTitle").isNotBlank()) builder.setEpisodeTitle(item.optString("episodeTitle").take(120))
                    }
                    val fingerprint = item.toString() + "|" + art
                    if (row != null && row.visible && fingerprints[key] == fingerprint) continue
                    val values = builder.build().toContentValues()
                    val programId = if (row == null || !row.visible) {
                        if (row != null) c.contentResolver.delete(uri(row.id), null, null)
                        val created = c.contentResolver.insert(TvContractCompat.WatchNextPrograms.CONTENT_URI, values) ?: continue
                        ContentUris.parseId(created)
                    } else {
                        c.contentResolver.update(uri(row.id), values, null, null)
                        row.id
                    }
                    p.edit().putString("row_$programId", key).putLong("rowPos_$programId", pos)
                        .putLong("rowTime_$programId", engaged).apply()
                    fingerprints[key] = fingerprint
                }
                for (row in rows.values) {
                    if (revision.get() != ticket) return@execute
                    if (row.key !in keep) {
                        c.contentResolver.delete(uri(row.id), null, null)
                        fingerprints.remove(row.key)
                        prefs(c).edit().remove("row_${row.id}").remove("rowPos_${row.id}").remove("rowTime_${row.id}").apply()
                    }
                }
                trimPreferences(c)
            } catch (e: Exception) {
                // Missing/disabled/denied TV provider must never break playback or launch.
                Log.w("StreamMaxWatchNext", "Watch Next indisponível (${e.javaClass.simpleName})")
            } finally {
                val active = resolvedScope
                val session = resolvedSession
                if (active != null) main.post { if (revision.get() == ticket) onScope(active, session) }
            }
        }
    }

    private fun uri(id: Long) = ContentUris.withAppendedId(TvContractCompat.WatchNextPrograms.CONTENT_URI, id)
    private fun rows(c: Context): List<Row> {
        val result = ArrayList<Row>()
        val projection = arrayOf("_id", "package_name", "internal_provider_id", "browsable", "last_playback_position_millis")
        c.contentResolver.query(TvContractCompat.WatchNextPrograms.CONTENT_URI, projection, null, null, null)?.use { cur ->
            while (cur.moveToNext()) {
                val key = cur.getString(2).orEmpty()
                if (cur.getString(1) == c.packageName && key.startsWith(PREFIX))
                    result.add(Row(cur.getLong(0), key, cur.getInt(3) != 0, cur.getLong(4)))
            }
        }
        return result
    }
    fun dismissed(context: Context, id: Long, done: () -> Unit) {
        val c = context.applicationContext
        worker.execute {
            try {
                val p = prefs(c)
                val key = p.getString("row_$id", null)
                if (id >= 0 && key?.startsWith(PREFIX) == true) p.edit()
                    .putLong("dismiss_$key", System.currentTimeMillis())
                    .putLong("dismissPos_$key", p.getLong("rowPos_$id", 0)).apply()
            } finally { done() }
        }
    }
    private fun trimPreferences(c: Context) {
        val p = prefs(c)
        val times = p.all.entries.filter { it.key.startsWith("dismiss_") && it.value is Long }
            .sortedByDescending { it.value as Long }
        val edit = p.edit()
        times.drop(96).forEach { edit.remove(it.key).remove("dismissPos_" + it.key.removePrefix("dismiss_")) }
        p.all.keys.filter { it.startsWith("row_") }.sortedByDescending { p.getLong("rowTime_" + it.removePrefix("row_"), 0) }
            .drop(96).forEach {
                val id = it.removePrefix("row_")
                edit.remove(it).remove("rowPos_$id").remove("rowTime_$id")
            }
        edit.apply()
    }
}

class StreamMaxWatchNextReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action != TvContractCompat.ACTION_WATCH_NEXT_PROGRAM_BROWSABLE_DISABLED) return
        val id = intent.getLongExtra(TvContractCompat.EXTRA_WATCH_NEXT_PROGRAM_ID, -1)
        if (id < 0) return
        val pending = goAsync()
        StreamMaxWatchNext.dismissed(context, id) { pending.finish() }
    }
}
