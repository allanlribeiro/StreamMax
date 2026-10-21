package com.streammax.tvdetox

import android.accessibilityservice.AccessibilityService
import android.content.Intent
import android.os.SystemClock
import android.view.accessibility.AccessibilityEvent

/**
 * Optional, user-enabled Google TV launcher redirect.
 * This does NOT intercept HOME keys. It reacts only when Google's launcher
 * foreground window is reported by the accessibility framework.
 */
class HomeRedirectAccessibilityService : AccessibilityService() {
    private var lastRedirectMs = 0L

    override fun onAccessibilityEvent(event: AccessibilityEvent?) {
        if (event?.eventType != AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) return
        if (event.packageName?.toString() != GOOGLE_TV_PACKAGE) return
        val preferences = getSharedPreferences(PREFS, MODE_PRIVATE)
        if (!preferences.getBoolean(ENABLED, false)) return

        val now = SystemClock.elapsedRealtime()
        if (now - lastRedirectMs < COOLDOWN_MS) return
        lastRedirectMs = now

        // Explicitly launch our activity. Do not change system package state.
        val intent = Intent(this, MainActivity::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_SINGLE_TOP)
        }
        runCatching { startActivity(intent) }
    }

    override fun onInterrupt() = Unit

    companion object {
        const val PREFS = "home_launcher_settings"
        const val ENABLED = "redirect_home_enabled"
        private const val GOOGLE_TV_PACKAGE = "com.google.android.apps.tv.launcherx"
        private const val COOLDOWN_MS = 1800L
    }
}
