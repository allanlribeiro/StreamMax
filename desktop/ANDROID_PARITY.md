# Windows parity contract — StreamMax Android 1.0.11 reference

## Source of truth
Android baseline: `feature/streammax-111-static-s-logo`. Windows development branch: `feature/streammax-windows-desktop`. No changes to Android until explicitly approved.

## Verified technical constraints
- Android app uses Kotlin, Android Gradle Plugin, Jetpack Compose and Media3; current `settings.gradle.kts` includes Android app and support modules, not a desktop module.
- Source entry point: `app/src/main/java/com/nuvio/tv/MainActivity.kt`; it includes navigation (`NavHost`), `AuthManager`, login via `AuthQrSignInScreen`, player dependencies, remote key event handling, and trailer-related behavior.
- The Electron `desktop/` preview is **not** a port of Android StreamMax. Its placeholder screen and local-file-only functionality are not acceptable as the Windows target.

## Acceptance contract: preserve Android experience
| Feature | Mandatory Windows acceptance |
| --- | --- |
| Home | Same visual hierarchy, cinematic hero/trailer behavior, rails, posters, scrolling and current design |
| Catalogs | Same account-backed catalogs, add-ons, search, details, source selection |
| Continue Watching | Same items, artwork and continuation behavior with persistence/sync where supported |
| Player | Same source flow, playback states, subtitles incl. pt-BR/pt-PT distinction, audio preferences, playback resume |
| Branding | Official S icon, same approved blue visual styling |
| Desktop adaptations | Mouse pointer, hover, click, wheel, keyboard Tab/Enter/Escape, video shortcuts, resizable window and F11 |
| Performance | Avoid unnecessary autoplay/decoding and provide reasonable desktop resource use |

## Engineering direction
1. Map Android screens, domain repositories and API contracts, with test fixtures/screenshots from 1.0.11.
2. Separate portable business logic from Android dependencies where possible, e.g. through Kotlin Multiplatform or service adapters; determine feasibility before committing to a technology.
3. Use a desktop rendering layer that reproduces current layout; do not invent a different product UX. Adapt only device-specific interactions.
4. Replace experimental Chromium video playback with a validated engine/adapter supporting actual supported sources and codecs; verify license/security implications.
5. Migrate features in the order login → catalogs/home → details/sources → player/subtitles → continue watching/settings.
6. Test visually and functionally on Windows against the same account and content as Android.

## Release gates
Do not label any Windows release 'complete' or 'equivalent to Android' until each mandatory acceptance row is implemented and tested. No installer build is evidence of feature parity. Current Windows 0.1.0 is only a tooling proof of concept.

## Immediate code audit to continue
Investigate the composables reached via `NavHost` in `MainActivity.kt`, then their repositories and player interfaces. Capture their state/data types without copying Android-only components into Electron verbatim. Confirm authentication and secrets do not rely on Android-only secure storage.

No changes to IPTV: the Windows port must follow the same scope decisions as approved for Android.
