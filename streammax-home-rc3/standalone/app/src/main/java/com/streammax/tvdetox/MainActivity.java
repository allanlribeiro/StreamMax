package com.streammax.tvdetox;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.provider.Settings;
import android.graphics.Color;
import android.view.Gravity;
import android.widget.*;
import java.util.*;
public final class MainActivity extends Activity {
 private LinearLayout apps;
 private int dp(int n){return (int)(n*getResources().getDisplayMetrics().density+.5f);}
 @Override public void onCreate(Bundle b){super.onCreate(b);
   LinearLayout root=new LinearLayout(this);root.setOrientation(1);root.setPadding(dp(32),dp(24),dp(32),dp(24));root.setBackgroundColor(0xff081528);
   TextView title=new TextView(this);title.setText("STREAMMAX HOME");title.setTextSize(26);title.setTextColor(Color.WHITE);root.addView(title);
   TextView section=new TextView(this);section.setText("Seus aplicativos");section.setTextSize(20);section.setTextColor(0xff69cfff);LinearLayout.LayoutParams sp=new LinearLayout.LayoutParams(-1,dp(65));sp.topMargin=dp(28);root.addView(section,sp);
   HorizontalScrollView scroll=new HorizontalScrollView(this);apps=new LinearLayout(this);apps.setOrientation(0);scroll.addView(apps);root.addView(scroll,new LinearLayout.LayoutParams(-1,dp(165)));
   Button settings=new Button(this);settings.setText("Configurar tela inicial");settings.setOnClickListener(v->showOptions());root.addView(settings);setContentView(root);
 }
 @Override protected void onResume(){super.onResume();loadApps();}
 private void loadApps(){apps.removeAllViews();PackageManager pm=getPackageManager();
  Intent query=new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER);
  List<ResolveInfo> found=pm.queryIntentActivities(query,0);
  if(found.isEmpty()) found=pm.queryIntentActivities(new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LAUNCHER),0);
  Collections.sort(found,new ResolveInfo.DisplayNameComparator(pm));
  Set<String> seen=new HashSet<>();
  for(ResolveInfo r:found){if(r.activityInfo==null)continue;
   String pkg=r.activityInfo.packageName;
   if(pkg.equals(getPackageName()) || !seen.add(pkg))continue;
   LinearLayout tile=new LinearLayout(this);tile.setOrientation(1);tile.setGravity(Gravity.CENTER);tile.setFocusable(true);tile.setBackgroundColor(0xff18304b);
   ImageView icon=new ImageView(this);icon.setImageDrawable(r.loadIcon(pm));tile.addView(icon,new LinearLayout.LayoutParams(dp(72),dp(72)));
   TextView name=new TextView(this);name.setText(r.loadLabel(pm));name.setTextColor(Color.WHITE);name.setGravity(Gravity.CENTER);name.setMaxLines(2);tile.addView(name);
   LinearLayout.LayoutParams lp=new LinearLayout.LayoutParams(dp(150),dp(142));lp.rightMargin=dp(12);apps.addView(tile,lp);
   tile.setOnFocusChangeListener((v,f)->v.setBackgroundColor(f?0xff216da0:0xff18304b));
   tile.setOnClickListener(v->{Intent launch=pm.getLeanbackLaunchIntentForPackage(pkg);
     if(launch==null)launch=pm.getLaunchIntentForPackage(pkg);
     if(launch==null) {launch=new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LEANBACK_LAUNCHER).setClassName(pkg,r.activityInfo.name);}
     try{startActivity(launch);}catch(RuntimeException e){Toast.makeText(this,"Não foi possível abrir",Toast.LENGTH_SHORT).show();}
   });
  }
  if(apps.getChildCount()>0)apps.getChildAt(0).requestFocus();
 }
 private void showOptions(){boolean enabled=getSharedPreferences("home",MODE_PRIVATE).getBoolean("override",false);
  new AlertDialog.Builder(this).setTitle("StreamMax como tela inicial").setMessage("Desative o redirecionamento do Projectivy antes do teste. Esta opção também requer sua autorização em Acessibilidade.")
  .setPositiveButton(enabled?"Desativar":"Ativar",(d,w)->getSharedPreferences("home",MODE_PRIVATE).edit().putBoolean("override",!enabled).apply())
  .setNeutralButton("Acessibilidade",(d,w)->startActivity(new Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS)))
  .setNegativeButton("Cancelar",null).show();
 }
}
