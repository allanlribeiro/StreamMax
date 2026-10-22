package com.streammax.tvdetox;

import android.accessibilityservice.AccessibilityService;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;

/**
 * Opt-in TX6 HOME helper for the ORIGINAL StreamMax Home RC2.
 * Never disables or modifies the manufacturer's launcher.
 * If RC2 is missing, key events are passed through unchanged.
 */
public final class HomeRedirectService extends AccessibilityService {
    private static final String TAG = "StreamMaxHomeTX6";
    private static final String FACTORY_HOME = "com.oranth.tvlauncher";
    private static final String RC2_PACKAGE = "com.streammax.home.rc2";
    private static final String RC2_ACTIVITY = "com.streammax.tvdetox.MainActivity";
    private long lastRedirect = -5000L;

    private boolean originalAvailable() {
        try {
            getPackageManager().getActivityInfo(new ComponentName(RC2_PACKAGE, RC2_ACTIVITY), 0);
            return true;
        } catch (PackageManager.NameNotFoundException ex) {
            return false;
        }
    }

    @Override public void onServiceConnected() {
        super.onServiceConnected();
        Log.i(TAG, "TX6 HOME helper connected; original launcher remains enabled");
    }

    @Override protected boolean onKeyEvent(KeyEvent event) {
        if (event == null || event.getKeyCode() != KeyEvent.KEYCODE_HOME) return false;
        if (!originalAvailable()) return false;
        if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
            return redirect("key");
        }
        return event.getAction() == KeyEvent.ACTION_UP;
    }

    @Override public void onAccessibilityEvent(AccessibilityEvent event) {
        if (event == null || event.getEventType() != AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) return;
        String pkg = event.getPackageName() == null ? "" : event.getPackageName().toString();
        if (FACTORY_HOME.equals(pkg) && originalAvailable()) redirect("window-fallback");
    }

    private boolean redirect(String reason) {
        long now = SystemClock.elapsedRealtime();
        if (now - lastRedirect < 2500) return true;
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.setClassName(RC2_PACKAGE, RC2_ACTIVITY);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        try {
            startActivity(intent);
            lastRedirect = now;
            Log.i(TAG, "Requested original RC2 via " + reason);
            return true;
        } catch (RuntimeException ex) {
            Log.e(TAG, "RC2 not launched, keeping original HOME", ex);
            return false;
        }
    }

    @Override public void onInterrupt() {
        Log.i(TAG, "TX6 HOME helper interrupted");
    }
}
