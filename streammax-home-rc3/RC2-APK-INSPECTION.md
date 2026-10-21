# APK RC2 — inspection evidence (2026-10-21)

Inspected the user-supplied `StreamMax-Home-1.0.0-RC2.apk` locally; **APK binary is not published in this repository**.

- SHA-256: `5633184b8e1c588abb3bd95c6eb10801573b3080202978a0764854b11a9d4213`
- Compressed APK: ~148 KB
- 6 ZIP entries, including `classes.dex`, `classes2.dex`, `AndroidManifest.xml`, `resources.arsc`
- AndroidManifest binary string pool includes:
  - Application ID: `com.streammax.home.rc2`
  - Main activity: `com.streammax.tvdetox.MainActivity`
  - Launch categories: `HOME`, `LAUNCHER`, `LEANBACK_LAUNCHER`
  - App label: `StreamMax Home 1.0 RC2`
- Background resource: `res/drawable-nodpi-v4/streammax_metal_waves.jpg`
- DEX app-owned classes include `MainActivity`, `Apps`, `LineIcon`, `Strings`.
- Build metadata: Android Gradle Plugin 8.7.3.

## Important next steps
1. Obtain original Java/Gradle project where possible. An APK is **compiled**, not interchangeable with source, and re-created UI may differ.
2. If rebuilding independently, preserve `com.streammax.home.rc2` only when **the original signing key** is available. Otherwise use a different application ID for safe side-by-side testing; do not remove RC2.
3. Integrate service/settings, preserve existing wallpaper, app shortcuts, launcher categories and remote navigation, then test on real TCL.
4. Validate that both the Projectivy override and its accessibility service are off when testing the RC3 redirect to avoid two competing accessibility redirectors.
5. New launcher installs need explicit user grant for accessibility. The service does not automatically become enabled by setting HOME role.

This evidence does not establish that the original RC2 source or keystore can be fully recovered from APK.
