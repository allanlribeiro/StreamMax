'use strict';
// Encrypted local playback metadata; same millisecond units as Android WatchProgress.
// No source URLs, tokens, credentials or subtitle file paths are ever persisted.
const fs=require('node:fs/promises');
const path=require('node:path');
const MAX_ITEMS=300;
function asText(value,max=250){
 return typeof value==='string'?value.slice(0,max):'';
}
function validNumber(value){
 return typeof value==='number'&&Number.isFinite(value)&&value>=0;
}
function sanitize(entry,clock=Date.now){
 if(!entry||typeof entry!=='object')throw new TypeError('Progress record required');
 const contentId=asText(entry.contentId,256).trim();
 const videoId=asText(entry.videoId,256).trim();
 const type=entry.contentType;
 if(!contentId||!videoId||!['movie','series'].includes(type))
  throw new TypeError('Invalid progress content identity');
 const season=type==='series'&&Number.isInteger(entry.season)&&entry.season>=0?entry.season:null;
 const episode=type==='series'&&Number.isInteger(entry.episode)&&entry.episode>=0?entry.episode:null;
 if(type==='series'&&(season===null||episode===null))
  throw new TypeError('Series episode identity required');
 if(!validNumber(entry.position)||!validNumber(entry.duration)||entry.duration<=0)
  throw new TypeError('Invalid progress duration');
 const position=Math.min(Math.round(entry.position),Math.round(entry.duration));
 const duration=Math.round(entry.duration);
 const lastWatched=validNumber(entry.lastWatched)?Math.round(entry.lastWatched):clock();
 return {
  contentId,contentType:type,videoId,season,episode,
  name:asText(entry.name,220),episodeTitle:asText(entry.episodeTitle,220),
  poster:asText(entry.poster,2048),backdrop:asText(entry.backdrop,2048),
  position,duration,lastWatched
 };
}
function progressKey(entry){
 return entry.contentType==='series'
  ?entry.contentId+'_s'+entry.season+'e'+entry.episode
  :entry.contentId;
}
function createWatchProgressStore({safeStorage,filePath,clock=Date.now}){
 if(!safeStorage||typeof safeStorage.isEncryptionAvailable!=='function')
  throw new TypeError('Windows protected storage required');
 if(typeof filePath!=='string'||!path.isAbsolute(filePath))
  throw new TypeError('Absolute progress path required');
 let work=Promise.resolve();
 function enqueue(action){
  const next=work.then(action);
  work=next.catch(()=>{});
  return next;
 }
 async function load(){
  if(!safeStorage.isEncryptionAvailable())throw new Error('Protected storage unavailable');
  let bytes;
  try{bytes=await fs.readFile(filePath);}
  catch(error){if(error.code==='ENOENT')return [];throw error;}
  const content=JSON.parse(safeStorage.decryptString(bytes));
  if(!content||content.version!==1||!Array.isArray(content.items))
   throw new Error('Invalid progress file');
  return content.items.slice(0,MAX_ITEMS).map(item=>sanitize(item,clock));
 }
 async function write(items){
  if(!safeStorage.isEncryptionAvailable())throw new Error('Protected storage unavailable');
  const limited=items.sort((a,b)=>b.lastWatched-a.lastWatched).slice(0,MAX_ITEMS);
  const encrypted=safeStorage.encryptString(JSON.stringify({version:1,items:limited}));
  await fs.mkdir(path.dirname(filePath),{recursive:true});
  const tmp=filePath+'.tmp';
  try{
   await fs.writeFile(tmp,encrypted,{mode:0o600,flag:'w'});
   await fs.rename(tmp,filePath);
  }catch(error){await fs.rm(tmp,{force:true}).catch(()=>{});throw error;}
 }
 return {
  upsert(entry){
   return enqueue(async()=>{
    const item=sanitize(entry,clock);
    const current=await load();
    const key=progressKey(item);
    const merged=current.filter(other=>progressKey(other)!==key);
    merged.push(item);
    await write(merged);
    return {saved:true};
   });
  },
  list(){
   return enqueue(async()=>{
    const items=await load();
    const latest=new Map();
    for(const item of items){
     const previous=latest.get(item.contentId);
     if(!previous||item.lastWatched>previous.lastWatched)latest.set(item.contentId,item);
    }
    // Match Android: select the most recent episode *before* excluding completed
    // or barely started items, so older episodes cannot reappear after completion.
    return [...latest.values()]
     .filter(item=>item.position/item.duration>=0.02&&item.position/item.duration<0.9)
     .sort((a,b)=>b.lastWatched-a.lastWatched);
   });
  },
  resume(contentId,videoId){
   return enqueue(async()=>{
    const item=(await load()).find(x=>x.contentId===contentId&&x.videoId===videoId);
    return item&&item.position/item.duration>=0.02&&item.position/item.duration<0.9
     ?{positionMs:item.position,durationMs:item.duration}:null;
   });
  },
  clear(){return enqueue(async()=>{await fs.rm(filePath,{force:true});return {cleared:true};});}
 };
}
module.exports={createWatchProgressStore,sanitize,progressKey};
