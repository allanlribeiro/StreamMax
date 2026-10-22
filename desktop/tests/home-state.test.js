const test = require('node:test');
const assert = require('node:assert/strict');
const { normalizeHomeState } = require('../home-state');
test('keeps Android layout selection and server rail order', () => {
  const source = {homeLayout:'MODERN',catalogRows:[{id:'a',title:'A',items:[1,2]},{id:'b',title:'B',items:[3]}],heroItems:[{id:9}],continueWatchingEnabled:true,continueWatchingItems:[{id:7}]};
  const result=normalizeHomeState(source);
  assert.equal(result.homeLayout,'MODERN');
  assert.deepEqual(result.catalogRows.map(x=>x.id),['a','b']);
  assert.deepEqual(result.heroItems,[{id:9}]);
  assert.deepEqual(result.continueWatchingItems,[{id:7}]);
});
test('does not expose disabled continue watching', () => {
  assert.deepEqual(normalizeHomeState({homeLayout:'CLASSIC',continueWatchingItems:[1]}).continueWatchingItems,[]);
});
test('rejects made-up layout or duplicated catalog identities', () => {
  assert.throws(()=>normalizeHomeState({homeLayout:'FANTASY'}));
  assert.throws(()=>normalizeHomeState({homeLayout:'GRID',catalogRows:[{id:'x',items:[]},{id:'x',items:[]}]}));
});

test('preserves home row order, grid items and poster/trailer preferences', () => {
  const source = {
    homeLayout: 'MODERN',
    homeRows: [{kind:'Catalog',row:{id:'first'}},{kind:'PlaceholderCatalog',catalogKey:'next'}],
    gridItems: [{kind:'Hero',items:[]}],
    heroSectionEnabled: false,
    modernLandscapePostersEnabled: true,
    focusedPosterBackdropTrailerEnabled: true,
    focusedPosterBackdropTrailerMuted: false,
    posterCardWidthDp: 140,
    posterCardHeightDp: 210
  };
  const state = normalizeHomeState(source);
  assert.deepEqual(state.homeRows, source.homeRows);
  assert.deepEqual(state.gridItems, source.gridItems);
  assert.equal(state.heroSectionEnabled, false);
  assert.equal(state.modernLandscapePostersEnabled, true);
  assert.equal(state.focusedPosterBackdropTrailerEnabled, true);
  assert.equal(state.focusedPosterBackdropTrailerMuted, false);
  assert.equal(state.posterCardWidthDp, 140);
  assert.equal(state.posterCardHeightDp, 210);
});

test('accepts Android CatalogRow identity fields without requiring invented id',()=>{
 const state=normalizeHomeState({
  homeLayout:'MODERN',
  catalogRows:[{
   addonId:'plugin',addonName:'Plugin',addonBaseUrl:'https://addon.example',
   catalogId:'popular',catalogName:'Filmes populares',type:'movie',items:[{id:'tt1',name:'Filme'}]
  }]
 });
 assert.equal(state.catalogRows.length,1);
 assert.equal(state.catalogRows[0].title,'Filmes populares');
 assert.equal(state.catalogRows[0].catalogId,'popular');
 assert.equal(state.catalogRows[0].items[0].id,'tt1');
});
