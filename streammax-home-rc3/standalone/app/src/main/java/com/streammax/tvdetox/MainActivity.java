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
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/** Lightweight TV launcher UI. HOME accessibility redirect is unchanged. */
public final class MainActivity extends Activity {
    private LinearLayout appRow;
    private final int navy = 0xff07101e;
    private final int panel = 0xff13243b;
    private final int cyan = 0xff58c7ff;

    private int dp(int n) {
        return (int) (n * getResources().getDisplayMetrics().density + 0.5f);
    }

    private GradientDrawable tileBackground(boolean focused) {
        GradientDrawable bg = new GradientDrawable(
            GradientDrawable.Orientation.TL_BR,
            focused ? new int[]{0xff235b92, 0xff143554} : new int[]{panel, 0xff0c1a2b}
        );
        bg.setCornerRadius(dp(18));
        bg.setStroke(dp(focused ? 3 : 1), focused ? cyan : 0xff29445e);
        return bg;
    }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(dp(38), dp(26), dp(38), dp(28));
        root.setBackground(new GradientDrawable(
            GradientDrawable.Orientation.TL_BR,
            new int[]{0xff0e2442, navy, 0xff090b17}
        ));

        TextView heading = new TextView(this);
        heading.setText("STREAMMAX");
        heading.setTextSize(31);
        heading.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        heading.setLetterSpacing(0.12f);
        heading.setTextColor(Color.WHITE);
        root.addView(heading);

        TextView tagline = new TextView(this);
        tagline.setText("Sua TV. Seu espaço.");
        tagline.setTextSize(15);
        tagline.setTextColor(0xffa4b8cf);
        root.addView(tagline);

        TextView section = new TextView(this);
        section.setText("APLICATIVOS");
        section.setLetterSpacing(0.14f);
        section.setTextSize(15);
        section.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        section.setTextColor(cyan);
        LinearLayout.LayoutParams sectionParams = new LinearLayout.LayoutParams(-1, dp(62));
        sectionParams.topMargin = dp(30);
        root.addView(section, sectionParams);

        HorizontalScrollView scroll = new HorizontalScrollView(this);
        scroll.setHorizontalScrollBarEnabled(false);
        scroll.setClipToPadding(false);
        appRow = new LinearLayout(this);
        appRow.setOrientation(LinearLayout.HORIZONTAL);
        appRow.setPadding(dp(2), dp(4), dp(4), dp(5));
        scroll.addView(appRow);
        root.addView(scroll, new LinearLayout.LayoutParams(-1, dp(182)));

        Button settings = new Button(this);
        settings.setText("Configurar tela inicial");
        settings.setOnClickListener(v -> showOptions());
        LinearLayout.LayoutParams buttonParams = new LinearLayout.LayoutParams(dp(260), dp(58));
        buttonParams.topMargin = dp(26);
        root.addView(settings, buttonParams);
        setContentView(root);
    }

    @Override protected void onResume() {
        super.onResume();
        populateApps();
    }

    private void populateApps() {
        appRow.removeAllViews();
        PackageManager pm = getPackageManager();
        Intent query = new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER);
        List<ResolveInfo> candidates = pm.queryIntentActivities(query, 0);
        if (candidates.isEmpty()) {
            candidates = pm.queryIntentActivities(
                new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LAUNCHER), 0
            );
        }
        Collections.sort(candidates, new ResolveInfo.DisplayNameComparator(pm));
        Set<String> seen = new HashSet<>();
        for (ResolveInfo app : candidates) {
            if (app.activityInfo == null) continue;
            String packageName = app.activityInfo.packageName;
            if (packageName.equals(getPackageName()) || !seen.add(packageName)) continue;

            LinearLayout tile = new LinearLayout(this);
            tile.setOrientation(LinearLayout.VERTICAL);
            tile.setGravity(Gravity.CENTER);
            tile.setPadding(dp(10), dp(12), dp(10), dp(9));
            tile.setFocusable(true);
            tile.setBackground(tileBackground(false));

            ImageView icon = new ImageView(this);
            icon.setImageDrawable(app.loadIcon(pm));
            icon.setScaleType(ImageView.ScaleType.FIT_CENTER);
            tile.addView(icon, new LinearLayout.LayoutParams(dp(76), dp(76)));

            TextView label = new TextView(this);
            label.setText(app.loadLabel(pm));
            label.setTextColor(Color.WHITE);
            label.setTextSize(14);
            label.setGravity(Gravity.CENTER);
            label.setMaxLines(2);
            LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(-1, dp(43));
            textParams.topMargin = dp(9);
            tile.addView(label, textParams);

            LinearLayout.LayoutParams tileParams = new LinearLayout.LayoutParams(dp(160), dp(155));
            tileParams.rightMargin = dp(14);
            appRow.addView(tile, tileParams);
            tile.setOnFocusChangeListener((view, focused) -> {
                view.setBackground(tileBackground(focused));
                view.setScaleX(focused ? 1.04f : 1f);
                view.setScaleY(focused ? 1.04f : 1f);
            });
            tile.setOnClickListener(view -> {
                Intent launch = pm.getLeanbackLaunchIntentForPackage(packageName);
                if (launch == null) launch = pm.getLaunchIntentForPackage(packageName);
                if (launch == null) {
                    launch = new Intent(Intent.ACTION_MAIN)
                        .addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER)
                        .setClassName(packageName, app.activityInfo.name);
                }
                try {
                    startActivity(launch);
                } catch (RuntimeException error) {
                    Toast.makeText(this, "Não foi possível abrir o aplicativo", Toast.LENGTH_SHORT).show();
                }
            });
        }
        if (appRow.getChildCount() > 0) appRow.getChildAt(0).requestFocus();
    }

    private void showOptions() {
        boolean enabled = getSharedPreferences("home", MODE_PRIVATE).getBoolean("override", false);
        new AlertDialog.Builder(this)
            .setTitle("StreamMax como tela inicial")
            .setMessage("O redirecionamento exige sua autorização em Acessibilidade. Para evitar conflito, desligue o redirecionamento do Projectivy.")
            .setPositiveButton(enabled ? "Desativar" : "Ativar",
                (dialog, which) -> getSharedPreferences("home", MODE_PRIVATE)
                    .edit().putBoolean("override", !enabled).apply())
            .setNeutralButton("Acessibilidade",
                (dialog, which) -> startActivity(new Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS)))
            .setNegativeButton("Cancelar", null)
            .show();
    }
}
