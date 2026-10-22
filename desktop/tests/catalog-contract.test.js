const test=require('node:test');
const assert=require('node:assert/strict');
const {catalogRequest}=require('../catalog-contract');
const base={addonBaseUrl:'https://example.org/addon/',addonId:'demo',addonName:'Addon',catalogId:'popular',catalogName:'Popular',type:'movie'};
test('keeps the Android repository defaults',()=>{
 const value=catalogRequest(base);
 assert.equal(value.skip,0);
 assert.equal(value.skipStep,100);
 assert.equal(value.posterScreen,'HOME');
 assert.equal(value.catalogId,'popular');
});
test('keeps paging and genre filter',()=>{
 const value=catalogRequest({...base,skip:100,skipStep:50,extraArgs:{genre:'horror'},supportsSkip:true});
 assert.equal(value.skip,100);
 assert.equal(value.extraArgs.genre,'horror');
 assert.equal(value.supportsSkip,true);
});
test('rejects negative pagination',()=>assert.throws(()=>catalogRequest({...base,skip:-1})));
