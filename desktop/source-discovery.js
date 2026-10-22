'use strict';
const {randomBytes}=require('node:crypto');
const {safeAddonUrl}=require('./catalog-client');
// A desktop implementation of incremental source discovery. Each configured
// addon returns independently; the slowest addon never blocks the first result.
// Playable URLs remain in Electron main, never crossing renderer IPC.
function streamUrl(base,type,id) {
 const url=safeAddonUrl(base);
 if(!/^[a-z0-9_-]{1,48}$/i.test(type)||typeof id!=='string'||!id||id.length>256)
  throw new TypeError('Invalid stream identity');
 url.pathname=url.pathname.replace(/\/$/,'').replace(/\/manifest\.json$/i,'')+
  '/stream/'+encodeURIComponent(type)+'/'+encodeURIComponent(id)+'.json';
 return url.href;
}
function createSourceDiscovery({transport=fetch,timeoutMs=10000}={}) {
 if(typeof transport!=='function')throw new TypeError('HTTP transport required');
 const sourceUrls=new Map();
 let generation=0;
 function reset(){generation++;sourceUrls.clear();}
 function getPlayableSource(sourceId){
  const entry=sourceUrls.get(sourceId);
  return entry&&entry.generation===generation?{...entry}:null;
 }
 function getPlayableUrl(sourceId){
  return getPlayableSource(sourceId)?.url||null;
 }
 async function search({type,id,providers,onUpdate=()=>{}}) {
  reset();
  const requestGeneration=generation;
  const idOfVideo=id;
  const eligible=(Array.isArray(providers)?providers:[]).filter(p=>p&&p.enabled!==false&&p.supportsStream!==false).slice(0,35);
  let completed=0,available=0;
  onUpdate({phase:'starting',total:eligible.length,completed,available,streams:[]});
  await Promise.all(eligible.map(async provider=>{
   const name=String(provider.name||'Addon').slice(0,120);
   let streams=[],error=null;
   try {
    const url=streamUrl(provider.baseUrl,type,id);
    const response=await transport(url,{
     method:'GET',headers:{Accept:'application/json'},redirect:'error',
     signal:AbortSignal.timeout(timeoutMs)
    });
    if(!response.ok)throw new Error('HTTP '+response.status);
    const body=await response.json();
    if(!body||!Array.isArray(body.streams))throw new TypeError('Invalid stream response');
    streams=body.streams.slice(0,100).map((entry,index)=>{
     if(!entry||typeof entry!=='object')return null;
     const title=String(entry.name||entry.title||'Fonte '+(index+1)).slice(0,240);
     const description=String(entry.description||'').slice(0,500);
     let playable=null;
     if(typeof entry.url==='string'){
      try {
       const candidate=new URL(entry.url);
       if(['http:','https:'].includes(candidate.protocol)&&!candidate.username&&!candidate.password)
        playable=candidate.href;
      }catch {}
     }
     const id=randomBytes(16).toString('hex');
     if(playable&&requestGeneration===generation)
      sourceUrls.set(id,{url:playable,generation:requestGeneration,type,videoId:idOfVideo});
     // Do not send sensitive stream URLs, infoHash or debrid query credentials to renderer.
     return {id,title,description,addonName:name,playable:Boolean(playable),kind:playable?'direct':'unsupported'};
    }).filter(Boolean);
   } catch(err) {
    error=err?.name==='TimeoutError'?'timeout':'unavailable';
   } finally {
    completed++;
    if(requestGeneration===generation){
     available+=streams.length;
     onUpdate({phase:completed===eligible.length?'complete':'searching',
      provider:name,total:eligible.length,completed,available,streams,error});
    }
   }
  }));
  return requestGeneration===generation
   ?{phase:'complete',total:eligible.length,completed,available}
   :{phase:'cancelled',total:eligible.length,completed,available:0};
 }
 return {search,reset,getPlayableUrl,getPlayableSource};
}
module.exports={createSourceDiscovery,streamUrl};
