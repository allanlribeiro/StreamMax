# Android 1.0.11 navigation audit for native Windows port

Source reference branch: `feature/streammax-111-static-s-logo`.

Verified files and entry points:
- `app/src/main/java/com/nuvio/tv/MainActivity.kt`: Android Compose Activity initializes the application, authentication gate, selected layout and `NuvioNavHost`.
- `app/src/main/java/com/nuvio/tv/ui/navigation/NuvioNavHost.kt`: routing and feature-to-feature transitions; home route around line 187, `HomeScreen` around 239, details route around 286, player around 881, search around 1150, library around 1192, settings around 1225.
- `app/src/main/java/com/nuvio/tv/ui/navigation/Screen.kt`: route structures and parameter contracts for details and playback.
- `app/src/main/java/com/nuvio/tv/core/auth/AuthManager.kt`: Supabase-based auth and TV QR/device login with Android-dependent calls.

Required sequence for a functional faithful port:
1. Document `HomeScreen` composable structure and backing view models, then implement pixel-matched desktop renderer from *the actual Android screen*, not the existing Electron welcome page.
2. Adapt Android authentication through equivalent secured Windows storage and compatible existing back-end; do not hardcode keys/tokens.
3. Port content repositories, addon handling, catalogs and details; preserve request/response contracts.
4. Port complete source selection and player workflow, incl. playback status, language variants and persisted choices.
5. Wire desktop mouse, keyboard, wheel, resize and full screen to those SAME flows.

The current `desktop/` Electron executable is only a proof of build/tooling; it does not implement these requirements. Do not release as StreamMax Windows equivalent.
