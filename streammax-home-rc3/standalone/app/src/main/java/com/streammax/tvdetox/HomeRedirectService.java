package com.streammax.tvdetox;

import android.accessibilityservice.AccessibilityService;
import android.content.Intent;
import android.os.SystemClock;
import android.util.Log;
import android.view.accessibility.AccessibilityEvent;

/** Opt-in launcher redirection; no system app is disabled. */
public final class HomeRedirectService extends AccessibilityService {
    private static final String TAG = "StreamMaxHomeRedirect";
    private static final String GOOGLE_HOME = "com.google.android.apps.tv.launcherx";
    private long lastRedirect;

    @Override public void onServiceConnected() {
        super.onServiceConnected();
        Log.i(TAG, "Accessibility service connected");
    }

    @Override public void onAccessibilityEvent(AccessibilityEvent event) {
        if (event == null || event.getEventType() != AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) return;
        String sourcePackage = event.getPackageName() == null ? "" : event.getPackageName().toString();
        boolean enabled = getSharedPreferences("home", MODE_PRIVATE).getBoolean("override", false);
        Log.d(TAG, "window=" + sourcePackage + " enabled=" + enabled);
        if (!GOOGLE_HOME.equals(sourcePackage) || !enabled) return;

        long now = SystemClock.elapsedRealtime();
        if (now - lastRedirect < 3000) return;
        lastRedirect = now;

        Intent intent = new Intent(this, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        try {
            startActivity(intent);
            Log.i(TAG, "Redirect requested from Google TV to StreamMax Home");
        } catch (RuntimeException ex) {
            Log.e(TAG, "Redirect failed", ex);
        }
    }

    @Override public void onInterrupt() {
        Log.w(TAG, "Accessibility service interrupted");
    }
}
