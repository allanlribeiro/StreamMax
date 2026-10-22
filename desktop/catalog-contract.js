'use strict';
// Mirrors the argument contract of Android CatalogRepository.getCatalog.
// The actual transport must be supplied by the authenticated add-on integration.
function catalogRequest(input) {
  if (!input || typeof input !== 'object' || Array.isArray(input)) throw new TypeError('Catalog request required');
  const required = ['addonBaseUrl','addonId','addonName','catalogId','catalogName','type'];
  const out = {};
  for (const key of required) {
    if (typeof input[key] !== 'string' || !input[key].trim()) throw new TypeError('Missing ' + key);
    out[key] = input[key].trim();
  }
  const u = new URL(out.addonBaseUrl);
  if (u.protocol !== 'https:') throw new TypeError('Add-on URL must use HTTPS');
  if (u.username || u.password) throw new TypeError('Credentials must not be embedded in URLs');
  const skip = input.skip ?? 0, skipStep = input.skipStep ?? 100;
  if (!Number.isSafeInteger(skip) || skip < 0 || !Number.isSafeInteger(skipStep) || skipStep <= 0) throw new TypeError('Invalid pagination');
  out.skip=skip;out.skipStep=skipStep;
  out.supportsSkip=input.supportsSkip===true;
  const extra=input.extraArgs ?? {};
  if (!extra || typeof extra!=='object' || Array.isArray(extra) || Object.values(extra).some(v=>typeof v!=='string')) throw new TypeError('Invalid extra arguments');
  out.extraArgs={...extra};
  out.posterScreen='HOME';
  return out;
}
module.exports={catalogRequest};
