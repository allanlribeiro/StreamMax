'use strict';
const fs=require('node:fs/promises');
const path=require('node:path');
function createSessionStore({safeStorage,filePath}){
 if(!safeStorage||typeof safeStorage.isEncryptionAvailable!=='function')throw new TypeError('Windows encryption provider required');
 if(!path.isAbsolute(filePath))throw new TypeError('Absolute session path required');
 function ready(){
  if(!safeStorage.isEncryptionAvailable())throw new Error('Windows protected encryption unavailable');
 }
 return {
  async save(tokens){
   ready();
   if(!tokens||typeof tokens.access_token!=='string'||!tokens.access_token||typeof tokens.refresh_token!=='string'||!tokens.refresh_token)throw new TypeError('Invalid session');
   const encrypted=safeStorage.encryptString(JSON.stringify(tokens));
   await fs.mkdir(path.dirname(filePath),{recursive:true});
   const temp=filePath+'.tmp';
   try{
    await fs.writeFile(temp,encrypted,{mode:0o600,flag:'w'});
    await fs.rename(temp,filePath);
   }catch(error){await fs.rm(temp,{force:true}).catch(()=>{});throw error;}
  },
  async load(){
   ready();
   let bytes;
   try{bytes=await fs.readFile(filePath);}catch(error){if(error.code==='ENOENT')return null;throw error;}
   const value=JSON.parse(safeStorage.decryptString(bytes));
   if(typeof value.access_token!=='string'||typeof value.refresh_token!=='string')throw new Error('Invalid stored session');
   return value;
  },
  async clear(){await fs.rm(filePath,{force:true});}
 };
}
module.exports={createSessionStore};
