package com.streammax.tvdetox;
import android.accessibilityservice.AccessibilityService;
import android.content.Intent;
import android.os.SystemClock;
import android.view.accessibility.AccessibilityEvent;
public final class HomeRedirectService extends AccessibilityService {
  private long last;
  @Override public void onAccessibilityEvent(AccessibilityEvent e) {
    if(e==null || e.getEventType()!=AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED || e.getPackageName()==null) return;
    if(!"com.google.android.apps.tv.launcherx".contentEquals(e.getPackageName())) return;
    if(!getSharedPreferences("home",MODE_PRIVATE).getBoolean("override",false)) return;
    long now=SystemClock.elapsedRealtime();
    if(now-last<3000) return;
    last=now;
    Intent i=new Intent(this,MainActivity.class).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_SINGLE_TOP);
    try {startActivity(i);}catch(RuntimeException ignored){}
  }
  @Override public void onInterrupt(){}
}
