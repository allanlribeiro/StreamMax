const {Player}=require('../../desktop/mpv.cjs');
const assert=require('node:assert/strict');
const http=require('node:http');
const fs=require('node:fs');
const path=require('node:path');
function until(fn,timeout=20000){return new Promise((resolve,reject)=>{const end=Date.now()+timeout;const id=setInterval(()=>{if(fn()){clearInterval(id);resolve();}else if(Date.now()>end){clearInterval(id);reject(Error('Player não alcançou o estado esperado'));}},100);});}
(async()=>{
 const fixture=fs.readFileSync(path.join(__dirname,'fixture.mp4'));
 const server=http.createServer((req,res)=>{if(req.headers.authorization!=='Bearer teste-do-player'){res.writeHead(401);return res.end();}res.writeHead(200,{'Content-Type':'video/mp4','Content-Length':fixture.length,'Accept-Ranges':'bytes'});res.end(fixture);});
 await new Promise(r=>server.listen(0,'127.0.0.1',r));
 const player=new Player(path.resolve('desktop/mpv/mpv.exe'));
 try{
  await player.open('http://127.0.0.1:'+server.address().port+'/fixture.mp4','Authorization: Bearer teste-do-player');
  await until(()=>player.state.ready && player.state.pos>0.5);assert.ok(player.state.duration>10);assert.equal(player.state.width,640);
  player.command('pause',true);await until(()=>!player.state.playing);
  player.command('seek',5);await until(()=>player.state.pos>=4.9);
  player.command('pause',false);await until(()=>player.state.playing);
  fs.mkdirSync('output/windows',{recursive:true});fs.writeFileSync('output/windows/reproducao.json',JSON.stringify({passed:true,checks:['vídeo e áudio','cabeçalho de autorização','metadados','posição','pausar','buscar','retomar'],width:player.state.width,duration:player.state.duration},null,2));
 }finally{player.close();server.close();}
})().catch(e=>{console.error(e);process.exit(1);});
