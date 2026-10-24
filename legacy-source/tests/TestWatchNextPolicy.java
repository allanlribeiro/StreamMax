import space.nuvio.nativelegacy.WatchNextPolicy;
public class TestWatchNextPolicy {
    public static void main(String[] args) {
        check(!WatchNextPolicy.unfinished(false, 10_000, 7_200_000, 95));
        check(!WatchNextPolicy.unfinished(false, 120_000, 7_200_000, 95));
        check(WatchNextPolicy.unfinished(false, 121_000, 7_200_000, 95));
        check(WatchNextPolicy.unfinished(false, 61_000, 2_000_000, 95));
        check(!WatchNextPolicy.unfinished(true, 60_000, 2_400_000, 95));
        check(WatchNextPolicy.unfinished(true, 121_000, 2_400_000, 95));
        check(!WatchNextPolicy.unfinished(true, 2_230_000, 2_400_000, 95));
        check(!WatchNextPolicy.unfinished(false, 6_900_000, 7_200_000, 95));
        check(!WatchNextPolicy.unfinished(false, 50, 0, 95));
        check(!WatchNextPolicy.unfinished(false, 50_000, Long.MAX_VALUE, 95));
        check(WatchNextPolicy.playUri("tt4154796", false, 0, 0).equals("streammax://play/movie/tt4154796"));
        check(WatchNextPolicy.playUri("tt0903747", true, 2, 3).endsWith("/series/tt0903747/2/3"));
        check(WatchNextPolicy.playUri("tt0903747", true, 0, 3).endsWith("/series/tt0903747"));
        check(WatchNextPolicy.playUri("tmdb:1399", true, 1, 1).contains("tmdb:1399/1/1"));
        check(WatchNextPolicy.playUri("https://private/?token=secret", false, 0, 0).isEmpty());
        check(WatchNextPolicy.playUri("tt123/../../other", false, 0, 0).isEmpty());
        String a = "a".repeat(64), b = "b".repeat(64);
        check(WatchNextPolicy.sameScope(a,a));
        check(!WatchNextPolicy.sameScope(a,b));
        check(!WatchNextPolicy.sameScope("", ""));
        check(WatchNextPolicy.publicArtwork("https://image.tmdb.org/t/p/w780/img.jpg").contains("w780"));
        for (String u : new String[]{"https://image.tmdb.org.evil/t.jpg", "http://image.tmdb.org/t.jpg",
                "https://local/cover?token=secret", "https://user:pass@image.tmdb.org/t.jpg",
                "https://image.tmdb.org/t.jpg?api_key=secret"}) check(WatchNextPolicy.publicArtwork(u).isEmpty());
        System.out.println("OK: Watch Next eligibility, credits, links, profile scope and public artwork");
    }
    private static void check(boolean value) { if (!value) throw new AssertionError(); }
}
