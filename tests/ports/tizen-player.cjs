const fs=require('node:fs');
const path=require('node:path');
const http=require('node:http');
async function tizenPlayer(page,out){
 const data=fs.readFileSync(path.join(__dirname,'fixture.mp4'));
 const server=http.createServer((req,res)=>{
  const range=/^bytes=(\d+)-(\d*)$/.exec(req.headers.range||'');
  if(range){const start=+range[1],end=range[2]?Math.min(+range[2],data.length-1):data.length-1;res.writeHead(206,{'Content-Type':'video/mp4','Content-Range':`bytes ${start}-${end}/${data.length}`,'Content-Length':end-start+1,'Accept-Ranges':'bytes'});return res.end(data.subarray(start,end+1));}
  res.writeHead(200,{'Content-Type':'video/mp4','Content-Length':data.length,'Accept-Ranges':'bytes'});res.end(data);
 });
 await new Promise(r=>server.listen(0,'0.0.0.0',r));
 try{
  const result=await page.evaluate(async url=>{
   if(!window.webapis||!webapis.avplay)throw Error('AVPlay não foi carregado');
   const p=webapis.avplay;
   try{
    p.open(url);p.setDisplayRect(0,0,1920,1080);
    await new Promise((resolve,reject)=>{const timer=setTimeout(()=>reject(Error('AVPlay não preparou a mídia')),30000);p.prepareAsync(()=>{clearTimeout(timer);resolve();},e=>{clearTimeout(timer);reject(Error(e.name||'Falha ao preparar'));});});
    p.play();await new Promise(r=>setTimeout(r,1600));
    const duration=p.getDuration(),pos=p.getCurrentTime();
    if(!(duration>10000 && pos>400))throw Error('AVPlay não avançou na reprodução');
    p.pause();if(p.getState()!=='PAUSED')throw Error('AVPlay não pausou');
    await new Promise((resolve,reject)=>{const timer=setTimeout(()=>reject(Error('AVPlay não concluiu a busca')),15000);p.seekTo(5000,()=>{clearTimeout(timer);resolve();},()=>{clearTimeout(timer);reject(Error('Falha ao buscar'));});});
    p.play();await new Promise(r=>setTimeout(r,800));
    if(p.getCurrentTime()<4500)throw Error('AVPlay não buscou o ponto solicitado');
    return {passed:true,durationMs:duration,positionMs:p.getCurrentTime(),checks:['SDK Samsung real','H.264/AAC por HTTP','reproduzir','pausar','buscar','retomar']};
   }finally{try{p.stop();p.close();}catch(_){}}
  },'http://10.0.2.2:'+server.address().port+'/fixture.mp4');
  fs.writeFileSync(out+'/reproducao.json',JSON.stringify(result,null,2));
 }finally{server.close();}
}
module.exports={tizenPlayer};
