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
