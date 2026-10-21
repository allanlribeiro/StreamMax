package com.streammax.home;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

/**
 * StreamMax Home Teste — no ads, no recommendations, no Internet permission.
 * First release is a launchable TEST APP, not a replacement for the TV HOME.
 * Root, ADB and package changes are intentionally out of scope.
 */
public final class HomeActivity extends Activity {
    private static final int WHITE = Color.rgb(239, 245, 252);
    private static final int MUTED = Color.rgb(172, 190, 207);
    private static final int CYAN = Color.rgb(99, 205, 247);

    private final Handler handler = new Handler(Looper.getMainLooper());
    private TextView clock, date, selection;
    private LinearLayout appStrip;
    private final Runnable ticker = new Runnable() {
        @Override public void run() {
            refreshTime();
            handler.postDelayed(this, 30000);
        }
    };

    private int dp(float n) {
        return (int) (n * getResources().getDisplayMetrics().density + 0.5f);
    }

    private TextView text(String value, float sp, int color, boolean bold) {
        TextView v = new TextView(this);
        v.setText(value);
        v.setTextColor(color);
        v.setTextSize(sp);
        v.setSingleLine(true);
        if (bold) v.setTypeface(null, android.graphics.Typeface.BOLD);
        return v;
    }

    private GradientDrawable bg(boolean focused, int radius) {
        GradientDrawable d = new GradientDrawable();
        d.setColor(focused ? 0xDB142F48 : 0xA60F1B2B);
        d.setCornerRadius(dp(radius));
        d.setStroke(dp(focused ? 2 : 1), focused ? CYAN : 0x66768A9B);
        return d;
    }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        FrameLayout root = new FrameLayout(this);
        root.setFocusableInTouchMode(true);
        root.addView(new Landscape(this),
            new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                                          ViewGroup.LayoutParams.MATCH_PARENT));

        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setPadding(dp(56), dp(34), dp(56), dp(42));
        root.addView(content, new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        LinearLayout top = new LinearLayout(this);
        top.setGravity(Gravity.TOP);
        top.setOrientation(LinearLayout.HORIZONTAL);
        content.addView(top, new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        LinearLayout greeting = new LinearLayout(this);
        greeting.setOrientation(LinearLayout.VERTICAL);
        top.addView(greeting, new LinearLayout.LayoutParams(0,
            ViewGroup.LayoutParams.WRAP_CONTENT, 1));

        TextView brand = text("STREAMMAX  /  HOME TESTE", 12, CYAN, true);
        brand.setLetterSpacing(0.18f);
        greeting.addView(brand);

        clock = text("--:--", 68, WHITE, false);
        LinearLayout.LayoutParams cl = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        cl.topMargin = dp(10);
        greeting.addView(clock, cl);

        date = text("", 17, MUTED, false);
        greeting.addView(date);

        LinearLayout topActions = new LinearLayout(this);
        topActions.setOrientation(LinearLayout.HORIZONTAL);
        topActions.setGravity(Gravity.RIGHT);
        top.addView(topActions);
        topActions.addView(action("ATUALIZAR", () -> loadApps()));
        topActions.addView(action("CONFIGURAÇÕES", () -> {
            try { startActivity(new Intent(Settings.ACTION_SETTINGS)); }
            catch (Exception e) {
                Toast.makeText(this, "Configurações indisponíveis nesta TV", Toast.LENGTH_SHORT).show();
            }
        }));

        View space = new View(this);
        content.addView(space, new LinearLayout.LayoutParams(1, 0, 1));

        LinearLayout dock = new LinearLayout(this);
        dock.setOrientation(LinearLayout.VERTICAL);
        content.addView(dock, new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        selection = text("SEUS APLICATIVOS", 22, WHITE, true);
        LinearLayout.LayoutParams st = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        st.bottomMargin = dp(8);
        dock.addView(selection, st);

        HorizontalScrollView scroll = new HorizontalScrollView(this);
        scroll.setHorizontalScrollBarEnabled(false);
        scroll.setClipChildren(false);
        scroll.setClipToPadding(false);
        scroll.setOverScrollMode(View.OVER_SCROLL_NEVER);
        dock.addView(scroll, new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, dp(150)));

        appStrip = new LinearLayout(this);
        appStrip.setOrientation(LinearLayout.HORIZONTAL);
        appStrip.setGravity(Gravity.CENTER_VERTICAL);
        appStrip.setPadding(dp(5), dp(10), dp(5), dp(10));
        appStrip.setClipChildren(false);
        appStrip.setClipToPadding(false);
        scroll.addView(appStrip, new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT));

        TextView footer = text("SEM ANÚNCIOS  •  SEM RECOMENDAÇÕES  •  OFFLINE", 11, MUTED, false);
        footer.setLetterSpacing(0.15f);
        LinearLayout.LayoutParams foot = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        foot.topMargin = dp(12);
        dock.addView(footer, foot);

        setContentView(root);
        refreshTime();
        loadApps();
    }

    private View action(String name, Runnable task) {
        TextView v = text(name, 12, WHITE, true);
        v.setGravity(Gravity.CENTER);
        v.setPadding(dp(16), 0, dp(16), 0);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, dp(43));
        lp.leftMargin = dp(12);
        v.setLayoutParams(lp);
        v.setBackground(bg(false, 22));
        v.setFocusable(true);
        v.setOnClickListener(view -> task.run());
        v.setOnFocusChangeListener((view, hasFocus) -> {
            v.setBackground(bg(hasFocus, 22));
            v.animate().scaleX(hasFocus ? 1.035f : 1).scaleY(hasFocus ? 1.035f : 1).setDuration(110).start();
        });
        return v;
    }

    private void refreshTime() {
        Date now = new Date();
        Locale locale = new Locale("pt", "BR");
        if (clock != null) {
            clock.setText(android.text.format.DateFormat.getTimeFormat(this).format(now));
        }
        if (date != null) {
            date.setText(new SimpleDateFormat("EEEE, d 'de' MMMM", locale).format(now));
        }
    }

    private void loadApps() {
        PackageManager pm = getPackageManager();
        Intent leanback = new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER);
        Intent normal = new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LAUNCHER);
        List<ResolveInfo> all = new ArrayList<>(pm.queryIntentActivities(leanback, 0));
        all.addAll(pm.queryIntentActivities(normal, 0));
        List<ResolveInfo> visible = new ArrayList<>();
        Set<String> seen = new HashSet<>();
        for (ResolveInfo app : all) {
            if (app.activityInfo == null || app.activityInfo.packageName == null) continue;
            String pkg = app.activityInfo.packageName;
            if (pkg.equals(getPackageName())) continue;
            if (pkg.equals("com.google.android.apps.tv.launcherx")
                    || pkg.equals("com.google.android.tvlauncher")) continue;
            if (seen.add(pkg)) visible.add(app);
        }

        visible.sort(Comparator
            .comparingInt((ResolveInfo app) -> {
                String pkg = app.activityInfo.packageName.toLowerCase(Locale.ROOT);
                return pkg.contains("streammax") ? 0 : 1;
            })
            .thenComparing(app -> {
                CharSequence label = app.loadLabel(pm);
                return label == null ? "" : label.toString().toLowerCase(Locale.ROOT);
            }));

        appStrip.removeAllViews();
        View first = null;
        for (ResolveInfo app : visible) {
            View tile = appTile(pm, app);
            if (first == null) first = tile;
            appStrip.addView(tile);
        }
        if (first != null) {
            first.requestFocus();
        } else {
            selection.setText("Nenhum aplicativo encontrado");
            TextView empty = text("Abra as configurações ou instale aplicativos Android TV.", 15, WHITE, false);
            appStrip.addView(empty);
        }
    }

    private View appTile(PackageManager pm, ResolveInfo entry) {
        String pkg = entry.activityInfo.packageName;
        CharSequence rawName = entry.loadLabel(pm);
        String name = rawName == null ? pkg : rawName.toString();

        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setGravity(Gravity.CENTER);
        card.setFocusable(true);
        card.setClickable(true);
        card.setContentDescription("Abrir " + name);
        card.setBackground(bg(false, 24));
        LinearLayout.LayoutParams itemParams = new LinearLayout.LayoutParams(dp(136), dp(118));
        itemParams.rightMargin = dp(14);
        card.setLayoutParams(itemParams);

        ImageView image = new ImageView(this);
        try {
            image.setImageDrawable(entry.loadIcon(pm));
        } catch (Exception ignored) {
            image.setImageResource(android.R.drawable.sym_def_app_icon);
        }
        image.setScaleType(ImageView.ScaleType.FIT_CENTER);
        card.addView(image, new LinearLayout.LayoutParams(dp(66), dp(66)));

        TextView label = text(name, 12, WHITE, true);
        label.setGravity(Gravity.CENTER);
        label.setMaxLines(1);
        label.setEllipsize(android.text.TextUtils.TruncateAt.END);
        LinearLayout.LayoutParams caption = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        caption.leftMargin = dp(5);
        caption.rightMargin = dp(5);
        caption.topMargin = dp(7);
        card.addView(label, caption);

        card.setOnFocusChangeListener((v, selected) -> {
            card.setBackground(bg(selected, 24));
            card.animate().scaleX(selected ? 1.07f : 1)
                .scaleY(selected ? 1.07f : 1).setDuration(130).start();
            if (selected) selection.setText(name);
        });
        card.setOnClickListener(v -> {
            try {
                Intent launch = pm.getLeanbackLaunchIntentForPackage(pkg);
                if (launch == null) launch = pm.getLaunchIntentForPackage(pkg);
                if (launch == null) throw new IllegalStateException("Sem atividade de abertura");
                startActivity(launch);
            } catch (Exception error) {
                Toast.makeText(this, "Não foi possível abrir " + name, Toast.LENGTH_LONG).show();
            }
        });
        card.setOnLongClickListener(v -> {
            try {
                Intent i = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                i.setData(android.net.Uri.parse("package:" + pkg));
                startActivity(i);
            } catch (Exception ignored) {
                Toast.makeText(this, "Detalhes indisponíveis", Toast.LENGTH_SHORT).show();
            }
            return true;
        });
        return card;
    }

    @Override protected void onResume() {
        super.onResume();
        handler.removeCallbacks(ticker);
        ticker.run();
    }

    @Override protected void onPause() {
        handler.removeCallbacks(ticker);
        super.onPause();
    }

    /** Static procedural wallpaper: no bitmap/video download, no network, no animation. */
    private static final class Landscape extends View {
        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        Landscape(Activity context) { super(context); }
        @Override protected void onDraw(Canvas c) {
            float w = getWidth(), h = getHeight();
            paint.setShader(new LinearGradient(0, 0, w, h,
                new int[]{0xFF0B1019, 0xFF132438, 0xFF080E16}, null, Shader.TileMode.CLAMP));
            c.drawRect(0, 0, w, h, paint);

            paint.setShader(new RadialGradient(w * 0.68f, h * 0.38f, w * 0.48f,
                new int[]{0x2255B7E6, 0x00162C42}, null, Shader.TileMode.CLAMP));
            c.drawRect(0, 0, w, h, paint);

            Path ridge = new Path();
            ridge.moveTo(0, h * 0.68f);
            ridge.cubicTo(w * 0.27f, h * 0.52f, w * 0.43f, h * 0.82f, w, h * 0.52f);
            ridge.lineTo(w, h);
            ridge.lineTo(0, h);
            ridge.close();
            paint.setShader(new LinearGradient(0, h * 0.5f, 0, h,
                0xFF17263A, 0xFF080D17, Shader.TileMode.CLAMP));
            c.drawPath(ridge, paint);

            Path front = new Path();
            front.moveTo(0, h * 0.83f);
            front.cubicTo(w * 0.39f, h * 0.72f, w * 0.51f, h * 0.95f, w, h * 0.78f);
            front.lineTo(w, h);
            front.lineTo(0, h);
            front.close();
            paint.setShader(new LinearGradient(0, h * 0.72f, 0, h,
                0xFF0C1422, 0xFF070B12, Shader.TileMode.CLAMP));
            c.drawPath(front, paint);
            paint.setShader(null);
        }
    }
}
