package space.nuvio.nativelegacy;

/** Pure policy shared by the Android publisher and its host-side tests. */
public final class WatchNextPolicy {
    private WatchNextPolicy() {}
    public static boolean unfinished(boolean series, long position, long duration, int completedPercent) {
        if (duration < 60_000 || duration > Integer.MAX_VALUE || position < 0 || position >= duration) return false;
        long started = series ? 120_000 : Math.min(120_000, duration * 3 / 100);
        long completed = duration * Math.max(50, Math.min(99, completedPercent)) / 100;
        // Approximate credits for TV episodes per Android's Watch Next guidelines.
        if (series && duration > 600_000) completed = Math.min(completed, duration - 180_000);
        return position > started && position < completed;
    }
    public static String playUri(String id, boolean series, int season, int episode) {
        if (id == null || !(id.matches("tt[0-9]{2,10}") || id.matches("tmdb:[1-9][0-9]{0,7}"))) return "";
        String base = "streammax://play/" + (series ? "series/" : "movie/") + id;
        if (series && season > 0 && season <= 9999 && episode > 0 && episode <= 99999)
            return base + "/" + season + "/" + episode;
        return base;
    }
    public static boolean sameScope(String expected, String current) {
        return expected != null && expected.matches("[a-f0-9]{64}") && expected.equals(current);
    }
    public static String publicArtwork(String value) {
        if (value == null || value.length() > 2048) return "";
        // Never expose addon/server credentials, local hosts or private media URLs.
        String[] hosts = {"image.tmdb.org", "images.metahub.space", "images.cinemeta.space",
                "artworks.thetvdb.com", "assets.fanart.tv"};
        for (String host : hosts)
            if (value.startsWith("https://" + host + "/") && value.indexOf('?') < 0 && value.indexOf('#') < 0)
                return value;
        return "";
    }
}
