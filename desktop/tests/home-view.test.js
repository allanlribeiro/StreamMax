const test=require('node:test');
const assert=require('node:assert/strict');
const {normalizeItem,safePosterUrl}=require('../home-view');
test('keeps only real content from catalog',()=>{
 assert.equal(normalizeItem({id:'',name:'Invalid'}),null);
 assert.equal(normalizeItem({id:'tt1',name:'Film One'}).name,'Film One');
 assert.equal(normalizeItem({id:'tt1',name:''}),null);
});
test('uses real metadata artwork only from HTTPS sources',()=>{
 const item=normalizeItem({id:'tt1',name:'Film',poster:'http://images.example/unsafe.jpg',background:'https://images.example/backdrop.jpg'});
 assert.equal(item.poster,'');
 assert.equal(item.background,'https://images.example/backdrop.jpg');
 assert.equal(safePosterUrl('javascript:alert(1)'),'');
 assert.equal(safePosterUrl('file:///etc/passwd'),'');
});
