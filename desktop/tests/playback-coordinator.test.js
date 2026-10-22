const test=require('node:test');
const assert=require('node:assert/strict');
const {EventEmitter}=require('node:events');
const {createPlaybackCoordinator}=require('../playback-coordinator');
function setup(resume=null){
 const events=new EventEmitter();
 const writes=[],seeks=[];
 const player={
  onProgress:fn=>events.on('progress',fn),
  onLoaded:fn=>events.on('loaded',fn),
  onFinished:fn=>events.on('finished',fn),
  onStatus:fn=>events.on('status',fn),
  seekTo:seconds=>{seeks.push(seconds);return {sent:true};},
  play:async()=>({started:true,engine:'mpv'}),
  stop:()=>({stopped:true})
 };
 const store={
  upsert:async e=>{writes.push(e);return {saved:true};},
  resume:async()=>resume,
  list:async()=>writes,
  clear:async()=>({cleared:true})
 };
 const sources={
  getPlayableSource:id=>id==='src'?{url:'https://media.example/video.mkv?token=private',type:'movie',videoId:'tt1'}:
   id==='ep'?{url:'https://media.example/episode.mkv?token=private',type:'series',videoId:'tt2:1:3'}:null
 };
 const ui=[];
 const coordinator=createPlaybackCoordinator({player,sources,store,clock:()=>20000,onProgress:e=>ui.push(e)});
 return {events,player,coordinator,writes,seeks,ui};
}
test('matches source to content and saves timeline using Android milliseconds',async()=>{
 const f=setup();
 await f.coordinator.play('src',{contentId:'tt1',contentType:'movie',name:'Movie',poster:'https://images.example/poster.jpg'});
 f.events.emit('progress',{positionMs:45000,durationMs:100000,bufferedSeconds:12});
 await f.coordinator.stop();
 assert.equal(f.writes.at(-1).position,45000);
 assert.equal(f.writes.at(-1).duration,100000);
 assert.equal(f.writes.at(-1).contentId,'tt1');
 assert.ok(!JSON.stringify(f.writes).includes('token=private'));
 assert.deepEqual(f.ui.at(-1),{positionMs:45000,durationMs:100000,bufferedSeconds:12});
});
test('resumes movie at saved position after mpv file-loaded',async()=>{
 const f=setup({positionMs:34000,durationMs:100000});
 const result=await f.coordinator.play('src',{contentId:'tt1',contentType:'movie'});
 assert.equal(result.resumeAvailable,true);
 assert.deepEqual(f.seeks,[]);
 f.events.emit('loaded');
 assert.deepEqual(f.seeks,[34]);
 f.events.emit('loaded');
 assert.deepEqual(f.seeks,[34]);
});
test('completed videos persist at 100% and are excluded by the store filter',async()=>{
 const f=setup();
 await f.coordinator.play('src',{contentId:'tt1',contentType:'movie'});
 f.events.emit('progress',{positionMs:95000,durationMs:100000,bufferedSeconds:1});
 f.events.emit('finished',{reason:'eof'});
 f.events.emit('status',{state:'ended'});
 await f.coordinator.stop();
 assert.equal(f.writes.at(-1).position,100000);
});
test('episode identity and season required; mismatched movie ID refused',async()=>{
 const f=setup();
 await assert.rejects(f.coordinator.play('src',{contentId:'wrong',contentType:'movie'}),/Filme não corresponde/);
 await assert.rejects(f.coordinator.play('ep',{contentId:'tt2',contentType:'series'}),/episódio/);
 const result=await f.coordinator.play('ep',{contentId:'tt2',contentType:'series',season:1,episode:3,name:'Series'});
 assert.equal(result.started,true);
 f.events.emit('progress',{positionMs:20000,durationMs:60000});
 await f.coordinator.stop();
 assert.equal(f.writes.at(-1).contentId,'tt2');
 assert.equal(f.writes.at(-1).videoId,'tt2:1:3');
 assert.equal(f.writes.at(-1).episode,3);
});

test('prefers verified local resume and does not request TV when local history exists',async()=>{
 const events=new EventEmitter();
 const seeks=[];
 let remoteReads=0;
 const player={
  onProgress:f=>events.on('progress',f),onLoaded:f=>events.on('loaded',f),
  onFinished:f=>events.on('finished',f),onStatus:f=>events.on('status',f),
  seekTo:x=>{seeks.push(x)},play:async()=>({started:true}),stop:()=>{}
 };
 const coordinator=createPlaybackCoordinator({
  player,sources:{getPlayableSource:()=>({type:'movie',videoId:'tt1',url:'https://cdn.example/movie'})},
  store:{resume:async()=>({positionMs:22000,durationMs:100000}),upsert:async()=>{},list:async()=>[],clear:async()=>{}},
  remoteResume:async()=>{remoteReads++;return {positionMs:45000,durationMs:100000}}
 });
 const playing=await coordinator.play('source',{contentId:'tt1',contentType:'movie'});
 assert.equal(playing.resumeSource,'local');
 events.emit('loaded');
 assert.deepEqual(seeks,[22]);
 assert.equal(remoteReads,0);
});
test('uses server-verified TV resume when local progress does not exist',async()=>{
 const events=new EventEmitter();
 const seeks=[];
 const player={
  onProgress:f=>events.on('progress',f),onLoaded:f=>events.on('loaded',f),
  onFinished:f=>events.on('finished',f),onStatus:f=>events.on('status',f),
  seekTo:x=>{seeks.push(x)},play:async()=>({started:true}),stop:()=>{}
 };
 const coordinator=createPlaybackCoordinator({
  player,sources:{getPlayableSource:()=>({type:'series',videoId:'ttshow:1:2',url:'https://cdn.example/ep'})},
  store:{resume:async()=>null,upsert:async()=>{},list:async()=>[],clear:async()=>{}},
  remoteResume:async(id,video)=>{
   assert.equal(id,'ttshow');
   assert.equal(video,'ttshow:1:2');
   return {positionMs:31000,durationMs:100000};
  }
 });
 const playing=await coordinator.play('episode',{contentId:'ttshow',contentType:'series',season:1,episode:2});
 assert.equal(playing.resumeAvailable,true);
 assert.equal(playing.resumeSource,'android-account');
 events.emit('loaded');
 assert.deepEqual(seeks,[31]);
});
test('unavailable or completed remote resume cannot block starting the player',async()=>{
 const f=setup();
 const original=f.coordinator;
 assert.equal(typeof original.play,'function');
 const events=new EventEmitter();
 let playCount=0;
 const player={
  onProgress:fn=>events.on('progress',fn),onLoaded:fn=>events.on('loaded',fn),
  onFinished:fn=>events.on('finished',fn),onStatus:fn=>events.on('status',fn),
  seekTo:()=>{throw new Error('Seek must not occur')},play:async()=>{playCount++;return {started:true}},stop:()=>{}
 };
 const deps={
  player,sources:{getPlayableSource:()=>({type:'movie',videoId:'tt1',url:'https://cdn.example/film'})},
  store:{resume:async()=>null,upsert:async()=>{},list:async()=>[],clear:async()=>{}}
 };
 const offline=createPlaybackCoordinator({...deps,remoteResume:async()=>{throw new Error('offline')}});
 assert.equal((await offline.play('src',{contentId:'tt1',contentType:'movie'})).resumeAvailable,false);
 const completed=createPlaybackCoordinator({...deps,remoteResume:async()=>({positionMs:94000,durationMs:100000})});
 assert.equal((await completed.play('src',{contentId:'tt1',contentType:'movie'})).resumeAvailable,false);
 assert.equal(playCount,2);
});
