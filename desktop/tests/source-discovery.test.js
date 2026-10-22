const test=require('node:test');
const assert=require('node:assert/strict');
const {createSourceDiscovery,streamUrl}=require('../source-discovery');
test('preserves configured addon paths and query parameters',()=>{
 assert.equal(streamUrl('https://addon.example/config?key=abc','movie','tt123'),
  'https://addon.example/config/stream/movie/tt123.json?key=abc');
 assert.throws(()=>streamUrl('http://addon.example','movie','tt123'),/Unsupported addon host/);
 assert.throws(()=>streamUrl('https://127.0.0.1','movie','tt123'),/Unsupported addon host/);
});
test('returns each addon independently without leaking playable URLs',async()=>{
 let releaseSlow;
 const slow=new Promise(resolve=>{releaseSlow=resolve;});
 const updates=[];
 const transport=async url=>{
  if(url.includes('/slow/')){await slow;return {ok:true,json:async()=>({streams:[{name:'Slow HD',url:'https://cdn.example/video2.mkv'}]})};}
  return {ok:true,json:async()=>({streams:[{name:'Fast 1080p',description:'Dublado',url:'https://cdn.example/video1.mkv'}]})};
 };
 const discovery=createSourceDiscovery({transport});
 const result=discovery.search({type:'movie',id:'tt123',providers:[
  {name:'Slow',baseUrl:'https://addon.example/slow',supportsStream:true},
  {name:'Fast',baseUrl:'https://addon.example/fast',supportsStream:true}
 ],onUpdate:update=>updates.push(update)});
 for(let i=0;i<12&&!updates.some(u=>u.provider==='Fast');i++)await new Promise(resolve=>setImmediate(resolve));
 const fast=updates.find(u=>u.provider==='Fast');
 assert.ok(fast,'Fast addon must publish before slow addon completes');
 assert.equal(fast.completed,1);
 assert.equal(fast.streams[0].title,'Fast 1080p');
 assert.equal(Object.hasOwn(fast.streams[0],'url'),false);
 assert.equal(discovery.getPlayableUrl(fast.streams[0].id),'https://cdn.example/video1.mkv');
 releaseSlow();
 assert.equal((await result).available,2);
 assert.equal(updates.at(-1).phase,'complete');
 discovery.reset();
 assert.equal(discovery.getPlayableUrl(fast.streams[0].id),null);
});
test('unsupported torrent descriptors stay visible but not playable',async()=>{
 const discovery=createSourceDiscovery({transport:async()=>({ok:true,json:async()=>({streams:[{name:'InfoHash',infoHash:'abc',sources:['tracker:a']} ]})})});
 const packets=[];
 await discovery.search({type:'movie',id:'tt1',providers:[{name:'Addon',baseUrl:'https://addon.example'}],onUpdate:u=>packets.push(u)});
 const stream=packets.at(-1).streams[0];
 assert.equal(stream.kind,'unsupported');
 assert.equal(stream.playable,false);
 assert.equal(discovery.getPlayableUrl(stream.id),null);
 assert.equal(Object.hasOwn(stream,'infoHash'),false);
});
