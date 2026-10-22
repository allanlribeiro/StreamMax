'use strict';
/**
 * Transport-neutral home state for the Windows port.
 * Mirrors HomeScreen.kt's Classic/Grid/Modern selector without inventing
 * catalog items, posters or playback sources.
 * This is a data boundary, NOT a complete Android UI implementation.
 */
const layouts = new Set(['CLASSIC', 'GRID', 'MODERN']);
function normalizeHomeState(payload) {
  if (!payload || typeof payload !== 'object' || Array.isArray(payload)) throw new TypeError('Home state required');
  const layout = String(payload.homeLayout ?? '').toUpperCase();
  if (!layouts.has(layout)) throw new TypeError('Unknown Android home layout');
  const rows = Array.isArray(payload.catalogRows) ? payload.catalogRows : [];
  const hero = Array.isArray(payload.heroItems) ? payload.heroItems : [];
  const watching = Array.isArray(payload.continueWatchingItems) ? payload.continueWatchingItems : [];
  const seen = new Set();
  const normalizedRows = rows.map((row, i) => {
    if (!row || typeof row !== 'object' || !Array.isArray(row.items)) throw new TypeError('Invalid catalog row at '+i);
    const id = String(row.id ?? row.key ?? (row.addonId && row.catalogId && (row.rawType || row.type) ? [row.addonId,row.addonBaseUrl || '',row.rawType || row.type,row.catalogId].join('::') : '')).trim();
    if (!id || seen.has(id)) throw new TypeError('Missing or duplicate catalog ID');
    seen.add(id);
    return { ...row, id, title: String(row.title ?? row.catalogName ?? ''), items: row.items.slice() };
  });
  return {
    homeLayout: layout,
    heroItems: hero.slice(),
    catalogRows: normalizedRows,
    continueWatchingEnabled: payload.continueWatchingEnabled === true,
    continueWatchingItems: payload.continueWatchingEnabled === true ? watching.slice() : [],
    homeRows: Array.isArray(payload.homeRows) ? payload.homeRows.slice() : [],
    gridItems: Array.isArray(payload.gridItems) ? payload.gridItems.slice() : [],
    heroSectionEnabled: payload.heroSectionEnabled !== false,
    modernLandscapePostersEnabled: payload.modernLandscapePostersEnabled === true,
    focusedPosterBackdropTrailerEnabled: payload.focusedPosterBackdropTrailerEnabled === true,
    focusedPosterBackdropTrailerMuted: payload.focusedPosterBackdropTrailerMuted !== false,
    posterCardWidthDp: Number.isFinite(payload.posterCardWidthDp) ? payload.posterCardWidthDp : 126,
    posterCardHeightDp: Number.isFinite(payload.posterCardHeightDp) ? payload.posterCardHeightDp : 189,
    posterCardCornerRadiusDp: Number.isFinite(payload.posterCardCornerRadiusDp) ? payload.posterCardCornerRadiusDp : 12,
    error: payload.error == null ? null : String(payload.error),
    isLoading: payload.isLoading === true
  };
}
module.exports = { normalizeHomeState };
