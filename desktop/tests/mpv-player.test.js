const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const os=require('node:os');
const path=require('node:path');
const {EventEmitter}=require('node:events');
const {createMpvPlayer,validateVideoUrl}=require('../mpv-player');

test('mpv launches without leaking signed stream URL in process command line',async()=>{
 const dir=fs.mkdtempSync(path.join(os.tmpdir(),'streammax-mpv-'));
 const exe=path.join(dir,'mpv.exe');
 fs.writeFileSync(exe,'test binary placeholder');
 let called=null,pipePath=null,socket;
 const player=createMpvPlayer({
  executable:exe,platform:'win32',randomId:()=> '0123456789abcdef0123456789abcdef',
  spawnProcess(binary,args,options){
   called={binary,args,options};
   const child=new EventEmitter();
   child.exitCode=null;
   child.killed=false;
   child.kill=()=>{child.killed=true;};
   return child;
  },
  connect(pipe){
   pipePath=pipe;
   const current=new EventEmitter();
   current.destroyed=false;
   current.messages=[];
   current.write=value=>current.messages.push(value);
   current.destroy=()=>{current.destroyed=true;};
   socket=current;
   queueMicrotask(()=>current.emit('connect'));
   return current;
  },
  sleep:async()=>{}
 });
 try{
  const secret='https://media.example/video.mkv?private=secret123';
  assert.equal(player.availability().available,true);
  const result=await player.play(secret);
  assert.equal(result.started,true);
  assert.equal(called.binary,exe);
  assert.equal(called.options.shell,false);
  assert.ok(!called.args.join(' ').includes('secret123'));
  assert.ok(called.args.some(x=>x.includes('--input-ipc-server=')));
  assert.ok(pipePath.includes('streammax-0123456789abcdef'));
  const commands=socket.messages.map(x=>JSON.parse(x).command);
  assert.ok(commands.some(x=>x[0]==='loadfile'&&x[1]===secret));
  assert.ok(commands.some(x=>x[0]==='observe_property'));
  socket.emit('data',Buffer.from('{"event":"file-loaded"}\n'));
  assert.equal(player.availability().state,'playing');
  assert.deepEqual(player.command('seekForward'),{sent:true});
  assert.ok(socket.messages.some(x=>x.includes('"seek",10,"relative"')));
  assert.throws(()=>player.command('run'),/Comando não suportado/);
  assert.deepEqual(player.stop(),{stopped:true});
  assert.equal(socket.destroyed,true);
  assert.equal(player.availability().state,'idle');
 }finally{fs.rmSync(dir,{recursive:true,force:true});}
});

test('rejects dangerous inputs and missing executable',async()=>{
 assert.throws(()=>validateVideoUrl('file:///C:/Windows/win.ini'));
 assert.throws(()=>validateVideoUrl('javascript:alert(1)'));
 assert.throws(()=>validateVideoUrl('https://user:password@example.com/movie.mkv'));
 const player=createMpvPlayer({platform:'win32'});
 await assert.rejects(player.play('https://example.com/movie.mp4'),/mpv.exe/);
 assert.throws(()=>player.setExecutable('C:\\not-mpv.exe'),/executável mpv.exe válido|mpv.exe não é um arquivo|ENOENT/);
});

test('does not expose playback URL in player events',async()=>{
 const dir=fs.mkdtempSync(path.join(os.tmpdir(),'streammax-mpv-'));
 const exe=path.join(dir,'mpv.exe');
 fs.writeFileSync(exe,'placeholder');
 const statuses=[];
 const player=createMpvPlayer({
  executable:exe,platform:'win32',
  spawnProcess(){
   const c=new EventEmitter();
   c.exitCode=null;c.kill=()=>{c.killed=true;};return c;
  },
  connect(){
   const s=new EventEmitter();
   s.destroyed=false;s.write=()=>{};
   s.destroy=()=>{s.destroyed=true;};
   queueMicrotask(()=>s.emit('connect'));
   return s;
  }
 });
 try{
  player.onStatus(p=>statuses.push(p));
  await player.play('https://cdn.example/movie.mp4?token=top-secret');
  assert.ok(statuses.length>=2);
  assert.ok(statuses.every(p=>!JSON.stringify(p).includes('top-secret')));
 }finally{player.stop();fs.rmSync(dir,{recursive:true,force:true});}
});
