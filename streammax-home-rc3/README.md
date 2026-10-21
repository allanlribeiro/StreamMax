# StreamMax Home RC3 — accessibility-based Home redirect (prototype)

This directory is an **integration prototype**, NOT an installable APK or compiled module.

## Why
On TCL Google TV, the system HOME role and preferred activity can point at StreamMax Home RC2 while pressing the remote's Home key still launches `com.google.android.apps.tv.launcherx`. Projectivy with its accessibility service enabled successfully redirects Home on the tested set.

## Integration into actual RC2 project
1. Obtain/verify the real StreamMax Home RC2 source, package, theme and MainActivity. The main StreamMax repository is the streaming app and must remain separate.
2. Copy `HomeRedirectAccessibilityService.kt` into the actual launcher module; ensure package and `MainActivity` match.
3. Merge `AndroidManifest.fragment.xml` service into the launcher manifest, add the XML resource and the label/description strings:
   - `home_redirect_accessibility_label`: `StreamMax Home — tela inicial`
   - `home_redirect_accessibility_description`: `Opcional: abre o StreamMax Home quando a tela inicial do Google TV aparecer.`
4. Expose a user-controlled toggle storing `home_launcher_settings/redirect_home_enabled` and a separate button that opens `Settings.ACTION_ACCESSIBILITY_SETTINGS`. Both the toggle **and** manual accessibility authorization must be enabled.
5. Disable Projectivy's override/accessibility *before testing RC3*, to prevent launch loops between competing redirectors.
6. Test remote Home, Back, Settings, boot, crash/restart, accessibility revocation and Google TV return on TCL. Restore behavior by disabling RC3 service or app toggle.
7. Only after on-TV validation build an APK signed with the same key and applicationId as RC2, or intentionally install side by side with a new ID.

## Safeguards
- Does not disable, uninstall or modify Google TV, Projectivy, system packages or remote/HDMI/Play services.
- No root or secure-settings writes.
- Responds only to Google's launcher window-state change events.
- Does not retrieve window contents.
- Throttles redirects; user can disable the service through TV Accessibility settings.
- This is a best-effort redirect, not a guaranteed HOME-key hook; TV firmware may behave differently.
