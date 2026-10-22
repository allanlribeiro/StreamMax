package com.streammax.tvdetox;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;

/** HOME bridge: launches the ORIGINAL, unmodified RC2; never paints an alternate layout. */
public final class MainActivity extends Activity {
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        Intent original = new Intent(Intent.ACTION_MAIN);
        original.setClassName("com.streammax.home.rc2", "com.streammax.tvdetox.MainActivity");
        original.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            startActivity(original);
        } catch (RuntimeException e) {
            Toast.makeText(this, "Instale o APK original StreamMax Home RC2", Toast.LENGTH_LONG).show();
        }
        finish();
    }
}
