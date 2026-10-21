package com.streammax.tvdetox

import android.content.Context
import android.content.Intent
import android.provider.Settings

/**
 * Small integration API for a switch/button in the real launcher settings screen.
 * This does not enable Accessibility programmatically; the user must approve it.
 */
object HomeRedirectSettings {
    fun isRedirectRequested(context: Context): Boolean =
        context.getSharedPreferences(
            HomeRedirectAccessibilityService.PREFS, Context.MODE_PRIVATE
        ).getBoolean(HomeRedirectAccessibilityService.ENABLED, false)

    fun setRedirectRequested(context: Context, enabled: Boolean) {
        context.getSharedPreferences(
            HomeRedirectAccessibilityService.PREFS, Context.MODE_PRIVATE
        ).edit().putBoolean(HomeRedirectAccessibilityService.ENABLED, enabled).apply()
    }

    fun openAccessibilitySettings(context: Context) {
        val intent = Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        }
        context.startActivity(intent)
    }
}
