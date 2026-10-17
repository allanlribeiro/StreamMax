# StreamMax

**StreamMax** is an independently branded Android TV project based on the open-source [Nuvio TV](https://github.com/NuvioMedia/NuvioTV) application. It is under active development.

The intended visual direction is **cinematic blue**: deep navy and black backgrounds, electric-blue/cyan accents, and subtle violet highlights. The project focuses on smooth remote-control navigation, **Continue Watching**, static artwork, and responsive performance on modest Android TV devices.

## Development and APK testing

The Android application is written with Kotlin, Jetpack Compose, TV Material 3, and Media3.

```bash
git clone https://github.com/allanlribeiro/StreamMax.git
cd StreamMax
./gradlew :app:assembleFullDebug
```

The `fullDebug` flavor is set up to use package name `com.streammax.tv.debug`. The `full` release flavor uses `com.streammax.tv`. Both are distinct from the upstream app's installation ID.

The repository's **PR Full Debug Build** GitHub Actions workflow generates a test APK, runs updater unit tests, and uploads the APK as a workflow artifact. To get the latest test build, open the repository's Actions tab, select a successful **PR Full Debug Build** run for the desired StreamMax changes, and download its artifact.

**Important:** A build is not a verified release. QR-code login depends on compatible backend settings, and installation/playback on actual TVs still require testing. Debug APK signing may differ between build environments.

## Project plan

See [STREAMMAX_PROJECT.md](./STREAMMAX_PROJECT.md) for the approved branding, current work and remaining tasks.

## Origins and license

StreamMax is a fork of [Nuvio TV](https://github.com/NuvioMedia/NuvioTV), licensed under the **GNU General Public License v3.0**. Attribution to the original project and upstream contributors is preserved. This fork is independent and is not an official Nuvio release.

See [LICENSE](./LICENSE) for license terms.
