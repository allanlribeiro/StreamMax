import space.nuvio.nativelegacy.StreamMaxVideoWindow;

public final class TestVideoWindow {
    private static void check(boolean value) { if (!value) throw new AssertionError(); }
    public static void main(String[] args) {
        // Caixa da versao 1.0.22: FIT deixa ~59 px logicos descobertos a direita.
        int[] fit = StreamMaxVideoWindow.calculate(555,0,1421,670,1920,1080,1);
        check(fit[0]+fit[2] < 1920-50);
        int[][] sizes={{1920,1080},{1920,804},{1080,1080},{720,1280},{3840,2160}};
        for (int[] s:sizes) for (int width:new int[]{1280,1920,3840}) {
            int x=width*28/100,w=width-x,h=width*670/1920;
            int[] r=StreamMaxVideoWindow.calculate(x,0,w,h,s[0],s[1],2);
            check(r[0]<=x && r[1]<=0 && r[0]+r[2]>=x+w && r[1]+r[3]>=h);
            check(Math.abs((double)r[2]/r[3]-(double)s[0]/s[1])<.01);
            // A mesma janela aceita metadata tardia sem passar por FIT.
            int[] pending=StreamMaxVideoWindow.calculate(x,0,w,h,0,0,2);
            check(pending[0]==x && pending[2]==w);
        }
        int[] exact=StreamMaxVideoWindow.calculate(-100,-50,2100,1200,1920,804,0);
        check(exact[0]==-100 && exact[1]==-50 && exact[2]==2100 && exact[3]==1200);
        System.out.println("OK: FIT reproduz faixa direita; COVER preenche, metadata tardia e EXACT preservados");
    }
}
