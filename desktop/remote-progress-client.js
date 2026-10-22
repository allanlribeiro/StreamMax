'use strict';
// Read-only compatibility bridge for Android WatchProgressSyncService.
// Intentionally does not mutate cloud data or merge it into local history.
function normalizeRemoteEntry(row){
 if(!row||typeof row!=='object')return null;
 const contentId=typeof row.content_id==='string'?row.content_id.trim():'';
 const videoId=typeof row.video_id==='string'?row.video_id.trim():'';
 const type=row.content_type;
 if(!contentId||contentId.length>256||!videoId||videoId.length>256||
  !['movie','series'].includes(type))return null;
 const season=Number.isInteger(row.season)&&row.season>=0?row.season:null;
 const episode=Number.isInteger(row.episode)&&row.episode>=0?row.episode:null;
 if(type==='series'&&(season===null||episode===null))return null;
 const number=value=>typeof value==='number'&&Number.isSafeInteger(value)&&value>=0?value:null;
 const duration=number(row.duration),position=number(row.position),lastWatched=number(row.last_watched);
 if(duration===null||duration===0||position===null||lastWatched===null)return null;
 return {
  contentId,contentType:type,videoId,season,episode,
  position:Math.min(position,duration),duration,lastWatched,
  // The RPC does not return artwork or titles. Never invent them.
  name:null,poster:null,backdrop:null,
  source:'android-account'
 };
}
function createRemoteProgressClient({sessionClient,endpoint,publicKey,profileId=1,transport=fetch}){
 if(!sessionClient||typeof sessionClient.withAccessToken!=='function')throw new TypeError('Authenticated session required');
 if(!Number.isInteger(profileId)||profileId<1)throw new TypeError('Invalid profile');
 const base=new URL(endpoint);
 if(base.protocol!=='https:'||base.username||base.password||base.search||base.hash)throw new TypeError('Secure endpoint required');
 const key=String(publicKey||'');
 if(!key)throw new TypeError('Public key required');
 const rpcUrl=new URL('rest/v1/rpc/sync_pull_watch_progress',base.href.replace(/\/?$/,'/')).href;
 let cache=[];
 let cacheUntil=0;
 let generation=0;
 async function list({limit=200}={}){
  if(!Number.isInteger(limit)||limit<1||limit>500)throw new TypeError('Invalid history limit');
  const requestGeneration=generation;
  return sessionClient.withAccessToken(async token=>{
   const response=await transport(rpcUrl,{
    method:'POST',redirect:'error',signal:AbortSignal.timeout(15000),
    headers:{apikey:key,Authorization:'Bearer '+token,'Content-Type':'application/json',Accept:'application/json'},
    body:JSON.stringify({p_profile_id:profileId,p_limit:limit})
   });
   if(!response.ok)throw new Error('Não foi possível consultar o histórico sincronizado ('+response.status+')');
   const body=await response.json();
   if(!Array.isArray(body))throw new Error('Resposta de histórico incompatível com o Android');
   const normalized=body.slice(0,limit).map(normalizeRemoteEntry).filter(Boolean);
   const latest=new Map();
   for(const item of normalized){
    const previous=latest.get(item.contentId);
    if(!previous||item.lastWatched>previous.lastWatched)latest.set(item.contentId,item);
   }
   const filtered=[...latest.values()]
    .filter(item=>item.position/item.duration>=0.02&&item.position/item.duration<0.90)
    .sort((a,b)=>b.lastWatched-a.lastWatched);
   if(requestGeneration!==generation)throw new Error('Consulta de histórico cancelada');
   cache=filtered;
   cacheUntil=Date.now()+5*60*1000;
   return filtered;
  });
 }
 return {
  list,
  async resume(contentId,videoId){
   if(typeof contentId!=='string'||typeof videoId!=='string'||!contentId||!videoId)return null;
   if(Date.now()>=cacheUntil){await list();}
   const entry=cache.find(x=>x.contentId===contentId&&x.videoId===videoId);
   return entry?{positionMs:entry.position,durationMs:entry.duration}:null;
  },
  clear(){generation++;cache=[];cacheUntil=0;}
 };
}
module.exports={normalizeRemoteEntry,createRemoteProgressClient};
