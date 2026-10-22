'use strict';
// Find existing mpv without running shell commands or silently downloading binaries.
// A bundled Windows player in extraResources/mpv/ has highest priority;
// explicit user selections are honored by the caller.
const fs=require('node:fs');
const path=require('node:path');

function candidatePaths({resourcesPath='',appDir='',env=process.env,pathModule=path}={}){
 const entries=[];
 const add=(dir,tail,origin)=>{
  if(typeof dir==='string'&&dir&&pathModule.isAbsolute(dir))
   entries.push({path:pathModule.join(dir,...tail),origin});
 };
 add(resourcesPath,['mpv','mpv.exe'],'bundled');
 add(appDir,['mpv','mpv.exe'],'portable');
 add(env.ProgramFiles,['mpv','mpv.exe'],'installed');
 add(env['ProgramFiles(x86)'],['mpv','mpv.exe'],'installed');
 add(env.LOCALAPPDATA,['Programs','mpv','mpv.exe'],'installed');
 add(env.LOCALAPPDATA,['Microsoft','WinGet','Links','mpv.exe'],'installed');
 add(env.USERPROFILE,['scoop','apps','mpv','current','mpv.exe'],'installed');
 add(env.ChocolateyInstall,['bin','mpv.exe'],'installed');
 return entries;
}
function findMpvExecutable({resourcesPath='',appDir='',env=process.env,fsModule=fs,pathModule=path}={}){
 for(const item of candidatePaths({resourcesPath,appDir,env,pathModule})){
  try{
   const stat=fsModule.statSync(item.path);
   if(stat.isFile()&&pathModule.basename(item.path).toLowerCase()==='mpv.exe')
    return {executable:item.path,source:item.origin};
  }catch{}
 }
 return {executable:null,source:'missing'};
}
module.exports={candidatePaths,findMpvExecutable};
