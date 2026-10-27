package space.nuvio.nativelegacy;

/** Geometry shared by real SurfaceView layout and the host regression tests. */
public final class StreamMaxVideoWindow {
    public static final int EXACT = 0, FIT = 1, COVER = 2;
    private StreamMaxVideoWindow() {}

    public static int[] calculate(int x, int y, int w, int h, int videoW, int videoH, int mode) {
        if (mode == EXACT || videoW <= 0 || videoH <= 0 || w <= 0 || h <= 0)
            return new int[]{x,y,w,h};
        int width, height;
        if (mode == COVER) {
            double scale = Math.max((double)w/videoW, (double)h/videoH);
            width = (int)Math.ceil(videoW*scale);
            height = (int)Math.ceil(videoH*scale);
        } else {
            // Mesma aritmetica float/arredondamento do FIT anterior.
            float scale = Math.min((float)w/videoW, (float)h/videoH);
            width = (int)(videoW*scale+0.5f);
            height = (int)(videoH*scale+0.5f);
        }
        return new int[]{x+(w-width)/2,y+(h-height)/2,width,height};
    }
}
