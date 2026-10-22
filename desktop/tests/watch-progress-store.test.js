const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs/promises');
const os=require('node:os');
const path=require('node:path');
const {createWatchProgressStore}=require('../watch-progress-store');

async function fixture(run){
 const folder=await fs.mkdtemp(path.join(os.tmpdir(),'streammax-history-'));
 const safeStorage={
  isEncryptionAvailable:()=>true,
  encryptString:value=>Buffer.from(value,'utf8'),
  decryptString:bytes=>Buffer.from(bytes).toString('utf8')
 };
 const filePath=path.join(folder,'history.enc');
 const store=createWatchProgressStore({safeStorage,filePath,clock:()=>1000});
 try{await run({store,filePath});}
 finally{await fs.rm(folder,{recursive:true,force:true});}
}
const movie=(position,duration=100000)=>({
 contentId:'tt100',contentType:'movie',videoId:'tt100',name:'My movie',
 poster:'https://images.example/poster.jpg',position,duration,lastWatched:1000
});
test('saves encrypted-style local history in Android milliseconds and resumes exact video',async()=>{
 await fixture(async({store,filePath})=>{
  await store.upsert(movie(34000));
  const raw=await fs.readFile(filePath);
  assert.ok(raw.length>0);
  assert.deepEqual(await store.resume('tt100','tt100'),{positionMs:34000,durationMs:100000});
  assert.equal((await store.list())[0].position,34000);
  assert.equal(await store.resume('tt100','other'),null);
 });
});
test('started threshold 2% and completed threshold 90% match Android',async()=>{
 await fixture(async({store})=>{
  await store.upsert(movie(1000));
  assert.deepEqual(await store.list(),[]);
  await store.upsert(movie(2000));
  assert.equal((await store.list()).length,1);
  await store.upsert(movie(90000));
  assert.deepEqual(await store.list(),[]);
  await store.upsert(movie(100000));
  assert.equal(await store.resume('tt100','tt100'),null);
 });
});
test('each episode has unique key and continuation shows latest series episode',async()=>{
 await fixture(async({store})=>{
  const base={contentId:'ttshow',contentType:'series',name:'My series',duration:100000};
  await store.upsert({...base,videoId:'ttshow:1:1',season:1,episode:1,position:25000,lastWatched:1});
  await store.upsert({...base,videoId:'ttshow:1:2',season:1,episode:2,position:35000,lastWatched:2});
  const items=await store.list();
  assert.equal(items.length,1);
  assert.equal(items[0].episode,2);
  assert.deepEqual(await store.resume('ttshow','ttshow:1:1'),{positionMs:25000,durationMs:100000});
 });
});
test('serialized concurrent writes do not lose episodes and signout clears records',async()=>{
 await fixture(async({store})=>{
  const base={contentId:'ttseries',contentType:'series',name:'Series',duration:100000};
  await Promise.all(Array.from({length:10},(_,n)=>store.upsert({
   ...base,videoId:'ttseries:1:'+(n+1),season:1,episode:n+1,position:30000,lastWatched:n+1
  })));
  assert.equal((await store.list())[0].episode,10);
  assert.deepEqual(await store.clear(),{cleared:true});
  assert.deepEqual(await store.list(),[]);
 });
});
test('invalid progress identity and duration rejected',async()=>{
 await fixture(async({store})=>{
  await assert.rejects(store.upsert({...movie(1000),duration:0}),/duration/);
  await assert.rejects(store.upsert({...movie(1000),contentType:'series'}),/episode identity/);
 });
});

test('completed latest episode suppresses stale continue-watching episode',async()=>{
 await fixture(async({store})=>{
  const base={contentId:'ttshow',contentType:'series',name:'Series',duration:100000};
  await store.upsert({...base,videoId:'ttshow:1:1',season:1,episode:1,position:45000,lastWatched:10});
  await store.upsert({...base,videoId:'ttshow:1:2',season:1,episode:2,position:95000,lastWatched:20});
  assert.deepEqual(await store.list(),[],'older episode must not reappear');
 });
});
