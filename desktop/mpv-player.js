'use strict';
// Windows mpv integration: URL is sent only through a random local named pipe.
// It never appears in argv, stdout logs, the renderer or shell commands.
const fs=require('node:fs');
const path=require('node:path');
const net=require('node:net');
const {spawn}=require('node:child_process');
const {randomBytes}=require('node:crypto');
const {EventEmitter}=require('node:events');

function validateExecutable(value){
 if(typeof value!=='string'||!path.isAbsolute(value)||path.basename(value).toLowerCase()!=='mpv.exe')
  throw new Error('Selecione um executável mpv.exe válido');
 const stat=fs.statSync(value);
 if(!stat.isFile())throw new Error('mpv.exe não é um arquivo');
 return value;
}
function validateVideoUrl(value){
 if(typeof value!=='string'||value.length>16384)throw new Error('Fonte inválida');
 const u=new URL(value);
 if(!['https:','http:'].includes(u.protocol)||u.username||u.password||u.hash)
  throw new Error('Protocolo da fonte não suportado pelo player');
 return u.href;
}
function createMpvPlayer({executable=null,spawnProcess=spawn,connect=net.createConnection,
 randomId=()=>randomBytes(20).toString('hex'),sleep=ms=>new Promise(r=>setTimeout(r,ms)),
 platform=process.platform}={}){
 const events=new EventEmitter();
 let child=null,socket=null,activeId=null,opening=false,buffer='',state='idle';
 let targetExe=executable;
 const publish=(next)=>{state=next;events.emit('status',{state:next});};
 function configured(){
  try{return !!validateExecutable(targetExe);}
  catch{return false;}
 }
 function send(command){
  if(!socket||socket.destroyed)throw new Error('A conexão com o player não está disponível');
  socket.write(JSON.stringify({command})+'\n','utf8');
 }
 function clearState(){
  activeId=null;
  if(socket){socket.destroy();socket=null;}
  if(child){const proc=child;child=null;if(proc.exitCode===null&&!proc.killed)proc.kill();}
  buffer='';
 }
 function consume(data,id){
  if(id!==activeId)return;
  buffer+=data.toString('utf8');
  if(buffer.length>524288){buffer='';return;}
  let newline;
  while((newline=buffer.indexOf('\n'))>=0){
   const line=buffer.slice(0,newline);buffer=buffer.slice(newline+1);
   let event;
   try{event=JSON.parse(line);}catch{continue;}
   if(event.event==='file-loaded')publish('playing');
   else if(event.event==='start-file')publish('loading');
   else if(event.event==='end-file')publish('ended');
   else if(event.event==='property-change'){
    if(event.name==='paused-for-cache'&&event.data===true)publish('buffering');
    else if(event.name==='pause'&&event.data===true)publish('paused');
    else if(event.name==='pause'&&event.data===false&&state!=='loading')publish('playing');
   }
  }
 }
 async function attach(pipe,id,processRef){
  let lastError=null;
  for(let attempt=0;attempt<35;attempt++){
   if(id!==activeId||processRef.exitCode!==null)throw new Error('Player encerrado durante a inicialização');
   try{
    const connection=await new Promise((resolve,reject)=>{
     let settled=false;
     const sock=connect(pipe);
     const done=(error)=>{
      if(settled)return;
      settled=true;
      sock.removeListener('connect',success);
      sock.removeListener('error',failure);
      if(error){sock.destroy();reject(error);}else resolve(sock);
     };
     const success=()=>done(null);
     const failure=error=>done(error);
     sock.once('connect',success);
     sock.once('error',failure);
    });
    if(id!==activeId){connection.destroy();throw new Error('Pesquisa de player cancelada');}
    return connection;
   }catch(error){lastError=error;await sleep(120);}
  }
  throw new Error('Não foi possível conectar ao mpv local'+(lastError?.code?' ('+lastError.code+')':''));
 }
 return {
  onStatus(listener){events.on('status',listener);return ()=>events.off('status',listener);},
  setExecutable(value){targetExe=validateExecutable(value);return {available:true};},
  availability(){return {available:configured(),state};},
  async play(videoUrl){
   if(platform!=='win32')throw new Error('Este player mpv está configurado para Windows');
   if(opening)throw new Error('O player já está iniciando');
   const url=validateVideoUrl(videoUrl);
   const binary=validateExecutable(targetExe);
   opening=true;
   clearState();
   const id=randomId();
   const pipe=String.raw`\\.\pipe\streammax-`+id;
   activeId=id;
   publish('starting');
   const args=['--no-config','--load-scripts=no','--ytdl=no',
    '--no-terminal','--idle=yes','--force-window=yes','--keep-open=no',
    '--input-ipc-server='+pipe];
   try{
    const proc=spawnProcess(binary,args,{shell:false,windowsHide:false,stdio:'ignore'});
    child=proc;
    proc.once('error',()=>{if(id===activeId)publish('error');});
    proc.once('exit',()=>{if(id===activeId){clearState();publish('idle');}});
    const link=await attach(pipe,id,proc);
    socket=link;
    socket.on('data',chunk=>consume(chunk,id));
    socket.on('error',()=>{if(id===activeId)publish('error');});
    socket.on('close',()=>{if(id===activeId&&state!=='idle')publish('disconnected');});
    send(['observe_property',1,'pause']);
    send(['observe_property',2,'paused-for-cache']);
    send(['loadfile',url,'replace']);
    publish('loading');
    return {started:true,engine:'mpv',state:'loading'};
   }catch(error){if(id===activeId){clearState();publish('error');}throw error;}
   finally{opening=false;}
  },
  command(name){
   const choices={
    pause:['cycle','pause'],seekBack:['seek',-10,'relative'],
    seekForward:['seek',10,'relative'],nextAudio:['cycle','audio'],
    nextSubtitle:['cycle','sub'],subtitleToggle:['cycle','sub-visibility']
   };
   if(!Object.hasOwn(choices,name))throw new TypeError('Comando não suportado');
   send(choices[name]);
   return {sent:true};
  },
  stop(){clearState();publish('idle');return {stopped:true};}
 };
}
module.exports={createMpvPlayer,validateExecutable,validateVideoUrl};
