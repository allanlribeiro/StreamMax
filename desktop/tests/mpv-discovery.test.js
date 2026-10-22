'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const path=require('node:path');
const {candidatePaths,findMpvExecutable}=require('../mpv-discovery');
test('bundled mpv has priority over system locations',()=>{
 const env={ProgramFiles:'C:\\Program Files',LOCALAPPDATA:'C:\\Users\\Test\\AppData\\Local',USERPROFILE:'C:\\Users\\Test'};
 const entries=candidatePaths({resourcesPath:'C:\\StreamMax\\resources',appDir:'C:\\StreamMax',env,
  pathModule:path.win32});
 assert.equal(entries[0].path,'C:\\StreamMax\\resources\\mpv\\mpv.exe');
 const result=findMpvExecutable({resourcesPath:'C:\\StreamMax\\resources',
  appDir:'C:\\StreamMax',env,pathModule:path.win32,
  fsModule:{statSync:p=>{if(p.includes('resources'))return {isFile:()=>true};throw Error('missing');}}});
 assert.equal(result.source,'bundled');
 assert.equal(result.executable,entries[0].path);
});
test('finds installed mpv without requiring manual configuration',()=>{
 const env={ProgramFiles:'C:\\Program Files',LOCALAPPDATA:'C:\\Users\\Test\\AppData\\Local'};
 const result=findMpvExecutable({resourcesPath:'C:\\StreamMax\\resources',env,pathModule:path.win32,
  fsModule:{statSync:p=>{
   if(p==='C:\\Program Files\\mpv\\mpv.exe')return {isFile:()=>true};
   throw Error('missing');
  }}});
 assert.equal(result.source,'installed');
 assert.equal(result.executable,'C:\\Program Files\\mpv\\mpv.exe');
});
test('does not pretend to have a player when mpv is missing',()=>{
 const result=findMpvExecutable({appDir:'C:\\StreamMax',env:{},pathModule:path.win32,
  fsModule:{statSync:()=>{throw Error('missing');}}});
 assert.deepEqual(result,{executable:null,source:'missing'});
});
