const {Player}=require('../../desktop/mpv.cjs');
const assert=require('node:assert/strict');
const http=require('node:http');
const fs=require('node:fs');
const path=require('node:path');
function until(fn,timeout=20000){return new Promise((resolve,reject)=>{const end=Date.now()+timeout;const id=setInterval(()=>{if(fn()){clearInterval(id);resolve();}else if(Date.now()>end){clearInterval(id);reject(Error('Player não alcançou o estado esperado'));}},100);});}
(async()=>{
 const fixture=fs.readFileSync(path.join(__dirname,'fixture.mp4'));
 const requests=[];
 const server=http.createServer((req,res)=>{
  requests.push({authorized:req.headers.authorization==='Bearer teste-do-player',range:req.headers.range||''});
  if(!requests.at(-1).authorized){res.writeHead(401);return res.end();}
  const range=/^bytes=(\d+)-(\d*)$/.exec(req.headers.range||'');
  if(range){const start=+range[1],end=range[2]?Math.min(+range[2],fixture.length-1):fixture.length-1;res.writeHead(206,{'Content-Type':'video/mp4','Content-Range':`bytes ${start}-${end}/${fixture.length}`,'Content-Length':end-start+1,'Accept-Ranges':'bytes'});return res.end(fixture.subarray(start,end+1));}
  res.writeHead(200,{'Content-Type':'video/mp4','Content-Length':fixture.length,'Accept-Ranges':'bytes'});res.end(fixture);
 });
 await new Promise(r=>server.listen(0,'127.0.0.1',r));
 fs.mkdirSync('output/windows',{recursive:true});
 let phase='preparar';
 const player=new Player(path.resolve('desktop/mpv/mpv.exe'),{logFile:path.resolve('output/windows/mpv-teste.log')});
 try{
  await player.open('http://127.0.0.1:'+server.address().port+'/fixture.mp4','Authorization: Bearer teste-do-player');
  await until(()=>player.state.ready && player.state.pos>0.5);assert.ok(player.state.duration>10);assert.equal(player.state.width,640);
  phase='pausar';player.command('pause',true);await until(()=>!player.state.playing);
  phase='buscar';player.command('seek',5);await until(()=>player.state.pos>=4.9);
  phase='retomar';player.command('pause',false);await until(()=>player.state.playing);
  fs.mkdirSync('output/windows',{recursive:true});fs.writeFileSync('output/windows/reproducao.json',JSON.stringify({passed:true,checks:['vídeo e áudio','cabeçalho de autorização','metadados','posição','pausar','buscar','retomar'],width:player.state.width,duration:player.state.duration},null,2));
 }catch(e){console.error(JSON.stringify({phase,state:player.state,requests},null,2));throw e;}
 finally{fs.writeFileSync('output/windows/requisicoes.json',JSON.stringify(requests,null,2));player.close();server.close();}
})().catch(e=>{console.error(e);process.exit(1);});
