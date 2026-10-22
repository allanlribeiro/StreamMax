'use strict';
// Coordinates the existing mpv player, source discovery and encrypted progress store.
// Source URL never crosses the renderer IPC; each saved entry matches Android's
// contentId / videoId / season / episode and millisecond position schema.
function createPlaybackCoordinator({player,sources,store,clock=Date.now,onProgress=()=>{},onError=()=>{}}){
 if(!player||!sources||!store)throw new TypeError('Playback dependencies required');
 let active=null;
 let latest={positionMs:0,durationMs:0,bufferedSeconds:null};
 let lastSaveAt=0, version=0;
 const validText=(x,max)=>typeof x==='string'?x.slice(0,max):'';
 function record(position=latest.positionMs,duration=latest.durationMs){
  if(!active||!Number.isFinite(position)||!Number.isFinite(duration)||duration<=0)return null;
  return {...active.metadata,videoId:active.videoId,
   position:Math.max(0,Math.round(Math.min(position,duration))),
   duration:Math.round(duration),lastWatched:clock()};
 }
 async function persist(force=false,finished=false){
  const snapshot=record(finished?latest.durationMs:latest.positionMs);
  if(!snapshot)return;
  if(!force&&clock()-lastSaveAt<12000)return;
  lastSaveAt=clock();
  try{await store.upsert(snapshot);}
  catch{onError('Não foi possível salvar o progresso local.');}
 }
 player.onProgress(event=>{
  if(!event||!active)return;
  if(Number.isFinite(event.positionMs)&&event.positionMs>=0)latest.positionMs=event.positionMs;
  if(Number.isFinite(event.durationMs)&&event.durationMs>=0)latest.durationMs=event.durationMs;
  latest.bufferedSeconds=typeof event.bufferedSeconds==='number'&&Number.isFinite(event.bufferedSeconds)
   ?event.bufferedSeconds:null;
  onProgress({...latest});
  if(latest.positionMs>0&&latest.durationMs>0)void persist(false);
 });
 player.onLoaded(()=>{
  if(!active||!active.resumeMs)return;
  const desired=active.resumeMs;
  active.resumeMs=0;
  try{player.seekTo(Math.min(desired/1000,Math.max(0,active.savedDurationMs/1000-15)));}
  catch{onError('Não foi possível retomar a posição anterior.');}
 });
 player.onFinished(({reason})=>{
  if(!active)return;
  void persist(true,reason==='eof');
 });
 player.onStatus(({state})=>{
  if(!active)return;
  if(['paused','ended','idle'].includes(state))void persist(true);
 });
 function metadataFrom(input,source){
  if(!input||typeof input!=='object')throw new TypeError('Informações do título obrigatórias');
  const contentId=validText(input.contentId,256).trim();
  const contentType=input.contentType;
  if(!contentId||!['movie','series'].includes(contentType)||contentType!==source.type)
   throw new TypeError('Identidade de reprodução incompatível');
  const season=contentType==='series'&&Number.isInteger(input.season)&&input.season>=0?input.season:null;
  const episode=contentType==='series'&&Number.isInteger(input.episode)&&input.episode>=0?input.episode:null;
  if(contentType==='series'&&(season===null||episode===null))
   throw new TypeError('Selecione o episódio antes de reproduzir');
  if(contentType==='movie'&&contentId!==source.videoId)
   throw new TypeError('Filme não corresponde à fonte');
  return {
   contentId,contentType,season,episode,
   name:validText(input.name,220),episodeTitle:validText(input.episodeTitle,220),
   poster:validText(input.poster,2048),backdrop:validText(input.backdrop,2048)
  };
 }
 return {
  async play(sourceId,metadata){
   const source=sources.getPlayableSource(sourceId);
   if(!source)throw new Error('Fonte expirada. Faça uma nova busca.');
   const meta=metadataFrom(metadata,source);
   // Preserve the previous title before changing playback.
   await persist(true);
   active=null;
   const myVersion=++version;
   latest={positionMs:0,durationMs:0,bufferedSeconds:null};
   lastSaveAt=0;
   let resume=null;
   try{resume=await store.resume(meta.contentId,source.videoId);}
   catch{onError('Histórico local indisponível: o vídeo começará do início.');}
   if(version!==myVersion)throw new Error('Reprodução substituída');
   active={metadata:meta,videoId:source.videoId,resumeMs:resume?.positionMs||0,
    savedDurationMs:resume?.durationMs||0};
   try{
    const result=await player.play(source.url);
    return {...result,resumeAvailable:!!resume};
   }catch(error){if(version===myVersion)active=null;throw error;}
  },
  async stop(){
   ++version;
   await persist(true);
   active=null;
   player.stop();
   latest={positionMs:0,durationMs:0,bufferedSeconds:null};
   return {stopped:true};
  },
  async signOut(){
   await this.stop();
   return store.clear();
  },
  list(){return store.list();},
  currentProgress(){return {...latest};},
  async resume(contentId,videoId){return store.resume(contentId,videoId);}
 };
}
module.exports={createPlaybackCoordinator};
