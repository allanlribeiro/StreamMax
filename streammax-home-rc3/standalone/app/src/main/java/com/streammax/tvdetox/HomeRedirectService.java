package com.streammax.tvdetox;

import android.accessibilityservice.AccessibilityService;
import android.content.Intent;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;

/** Opt-in HOME interception experiment and fallback launcher redirection. */
public final class HomeRedirectService extends AccessibilityService {
    private static final String TAG = "StreamMaxHomeRedirect";
    private static final String GOOGLE_HOME = "com.google.android.apps.tv.launcherx";
    // Critical: NEVER launch the re-created RC3/RC4 layout. Open unmodified RC2.
    private long lastRedirect;

    private boolean enabled() {
        // Enabling the AccessibilityService in Android settings is the explicit opt-in.\n        return true;
    }

    @Override public void onServiceConnected() {
        super.onServiceConnected();
        Log.i(TAG, "Accessibility service connected; testing HOME key filtering");
    }

    @Override protected boolean onKeyEvent(KeyEvent event) {
        if (!enabled() || event.getKeyCode() != KeyEvent.KEYCODE_HOME) return false;
        Log.i(TAG, "HOME key seen action=" + event.getAction());
        if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
            redirect("key");
        }
        return true;
    }

    @Override public void onAccessibilityEvent(AccessibilityEvent event) {
        if (event == null || event.getEventType() != AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) return;
        String pkg = event.getPackageName() == null ? "" : event.getPackageName().toString();
        boolean requested = enabled();
        Log.d(TAG, "window=" + pkg + " enabled=" + requested);
        if (GOOGLE_HOME.equals(pkg) && requested) redirect("window-fallback");
    }

    private void redirect(String trigger) {
        long now = SystemClock.elapsedRealtime();
        if (now - lastRedirect < 2500) return;
        lastRedirect = now;
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.setClassName("com.streammax.home.rc2", "com.streammax.tvdetox.MainActivity");
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        try {
            startActivity(intent);
            Log.i(TAG, "Original RC2 launch requested trigger=" + trigger);
        } catch (RuntimeException ex) {
            Log.e(TAG, "Redirect failed trigger=" + trigger, ex);
        }
    }

    @Override public void onInterrupt() {
        Log.w(TAG, "Accessibility service interrupted");
    }
}
