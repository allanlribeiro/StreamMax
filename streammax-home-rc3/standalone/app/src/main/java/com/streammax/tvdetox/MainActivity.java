package com.streammax.tvdetox;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.provider.Settings;
import android.view.Gravity;
import android.view.View;
import android.widget.*;
import java.util.*;

/**
 * Reconstructed RC2-inspired "StreamMax Metal" layout:
 * full-screen dark metallic background; clock; status; two app rows;
 * settings/quick actions; TV remote focus and app launch.
 *
 * Original RC2 source is unavailable: this is a functional reconstruction,
 * not a byte-for-byte reproduction. The AccessibilityService is untouched.
 */
public final class MainActivity extends Activity {
    private LinearLayout topApps, bottomApps;
    private TextClock clock;
    private int dp(int px) { return (int)(px*getResources().getDisplayMetrics().density+0.5f); }
    private GradientDrawable fill(int color, int border) {
        GradientDrawable d = new GradientDrawable();
        d.setColor(color); d.setCornerRadius(dp(15)); d.setStroke(dp(1),border); return d;
    }
    private TextView text(String value,int size,int color) {
        TextView t=new TextView(this);t.setText(value);t.setTextColor(color);t.setTextSize(size);return t;
    }
    private LinearLayout horizontal() {
        LinearLayout l=new LinearLayout(this);l.setOrientation(LinearLayout.HORIZONTAL);l.setGravity(Gravity.CENTER_VERTICAL);return l;
    }
    private void spacer(LinearLayout l,int height) {
        l.addView(new View(this),new LinearLayout.LayoutParams(1,dp(height)));
    }
    @Override public void onCreate(Bundle saved) {
        super.onCreate(saved);
        // Prefer the extracted RC2 wallpaper if placed in the drawable folder.
        LinearLayout root=new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(dp(42),dp(24),dp(42),dp(24));
        root.setBackground(new GradientDrawable(GradientDrawable.Orientation.BL_TR,
            new int[]{0xff04070c,0xff13283a,0xff04060c}));
        setContentView(root);

        LinearLayout header=horizontal();
        LinearLayout titleBlock=new LinearLayout(this);titleBlock.setOrientation(1);
        TextView brand=text("STREAMMAX",28,Color.WHITE);
        brand.setTypeface(Typeface.DEFAULT,Typeface.BOLD);brand.setLetterSpacing(.08f);
        titleBlock.addView(brand);
        TextView sub=text("HOME  •  METAL",12,0xff8fa9bd);sub.setLetterSpacing(.14f);titleBlock.addView(sub);
        header.addView(titleBlock,new LinearLayout.LayoutParams(0,dp(70),1));

        clock=new TextClock(this);clock.setFormat12Hour("HH:mm");clock.setFormat24Hour("HH:mm");
        clock.setTextSize(38);clock.setTextColor(Color.WHITE);clock.setGravity(Gravity.RIGHT);
        header.addView(clock,new LinearLayout.LayoutParams(dp(155),dp(56)));
        root.addView(header);
        TextView network=text("Acesso rápido aos seus aplicativos",14,0xffb2c5d6);root.addView(network);
        spacer(root,35);

        TextView section=text("APLICATIVOS",16,Color.WHITE);section.setTypeface(Typeface.DEFAULT,Typeface.BOLD);
        section.setLetterSpacing(.12f);root.addView(section);
        spacer(root,15);
        topApps=horizontal();
        HorizontalScrollView first=new HorizontalScrollView(this);first.setHorizontalScrollBarEnabled(false);
        first.addView(topApps);root.addView(first,new LinearLayout.LayoutParams(-1,dp(153)));
        spacer(root,18);
        bottomApps=horizontal();
        HorizontalScrollView second=new HorizontalScrollView(this);second.setHorizontalScrollBarEnabled(false);
        second.addView(bottomApps);root.addView(second,new LinearLayout.LayoutParams(-1,dp(153)));
        spacer(root,25);

        LinearLayout quick=horizontal();
        addQuick(quick,"⚙  Ajustes",()->startActivity(new Intent(Settings.ACTION_SETTINGS)));
        addQuick(quick,"◉  Wi-Fi",()->startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS)));
        addQuick(quick,"⌂  Tela inicial",this::showOptions);
        root.addView(quick);
    }
    private void addQuick(LinearLayout parent,String name,Runnable action) {
        TextView v=text(name,15,Color.WHITE);v.setGravity(Gravity.CENTER);v.setFocusable(true);
        v.setBackground(fill(0xaa1c2a3b,0xff607b91));
        LinearLayout.LayoutParams p=new LinearLayout.LayoutParams(dp(185),dp(49));p.rightMargin=dp(14);
        parent.addView(v,p);
        v.setOnClickListener(w->action.run());
        v.setOnFocusChangeListener((w,f)->v.setBackground(fill(f?0xff246994:0xaa1c2a3b,f?0xff80d5ff:0xff607b91)));
    }
    @Override protected void onResume() {super.onResume();populateApps();}
    private void populateApps() {
        topApps.removeAllViews();bottomApps.removeAllViews();
        PackageManager pm=getPackageManager();
        Intent intent=new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER);
        List<ResolveInfo> found=pm.queryIntentActivities(intent,0);
        if(found.isEmpty()) found=pm.queryIntentActivities(
            new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LAUNCHER),0);
        Collections.sort(found,new ResolveInfo.DisplayNameComparator(pm));
        Set<String> seen=new HashSet<>();int index=0;
        for(ResolveInfo r:found) {
            if(r.activityInfo==null) continue;
            String pkg=r.activityInfo.packageName;
            if(pkg.equals(getPackageName()) || !seen.add(pkg)) continue;
            LinearLayout target=(index++%2==0)?topApps:bottomApps;
            LinearLayout tile=new LinearLayout(this);
            tile.setOrientation(1);tile.setGravity(Gravity.CENTER);tile.setFocusable(true);
            tile.setPadding(dp(8),dp(10),dp(8),dp(6));tile.setBackground(fill(0xbb142436,0xff30475a));
            ImageView icon=new ImageView(this);icon.setImageDrawable(r.loadIcon(pm));
            tile.addView(icon,new LinearLayout.LayoutParams(dp(70),dp(70)));
            TextView label=text(String.valueOf(r.loadLabel(pm)),13,Color.WHITE);
            label.setMaxLines(2);label.setGravity(Gravity.CENTER);
            LinearLayout.LayoutParams labelParams=new LinearLayout.LayoutParams(-1,dp(40));
            tile.addView(label,labelParams);
            LinearLayout.LayoutParams params=new LinearLayout.LayoutParams(dp(142),dp(136));
            params.rightMargin=dp(13);target.addView(tile,params);
            tile.setOnFocusChangeListener((v,f)->{
                v.setBackground(fill(f?0xff245578:0xbb142436,f?0xff61caff:0xff30475a));
                v.setScaleX(f?1.04f:1f);v.setScaleY(f?1.04f:1f);
            });
            tile.setOnClickListener(v->{
                Intent launch=pm.getLeanbackLaunchIntentForPackage(pkg);
                if(launch==null)launch=pm.getLaunchIntentForPackage(pkg);
                if(launch==null)launch=new Intent(Intent.ACTION_MAIN)
                    .addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER)
                    .setClassName(pkg,r.activityInfo.name);
                try {startActivity(launch);}
                catch(RuntimeException e){Toast.makeText(this,"Não foi possível abrir",Toast.LENGTH_SHORT).show();}
            });
        }
        if(topApps.getChildCount()>0)topApps.getChildAt(0).requestFocus();
    }
    private void showOptions() {
        boolean enabled=getSharedPreferences("home",MODE_PRIVATE).getBoolean("override",false);
        new AlertDialog.Builder(this).setTitle("StreamMax como tela inicial")
            .setMessage("Recurso opcional. Acessibilidade deve ser autorizada manualmente.")
            .setPositiveButton(enabled?"Desativar":"Ativar",
                (d,w)->getSharedPreferences("home",MODE_PRIVATE).edit().putBoolean("override",!enabled).apply())
            .setNeutralButton("Acessibilidade",
                (d,w)->startActivity(new Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS)))
            .setNegativeButton("Cancelar",null).show();
    }
}
