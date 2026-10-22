'use strict';
// Desktop connection settings are PUBLIC endpoint configuration, never session tokens.
// Session tokens continue to use Electron safeStorage in session-store.js.
const fs=require('node:fs');
const path=require('node:path');
function normalizeServerConfig(input){
 if(!input||typeof input!=='object')throw new TypeError('Configuração do servidor inválida');
 const endpoint=String(input.endpoint||'').trim().replace(/\/+$/,'');
 const publicKey=String(input.publicKey||'').trim();
 const redirectUrl=String(input.redirectUrl||'').trim();
 if(endpoint.length>2048||redirectUrl.length>2048||publicKey.length>4096)
  throw new Error('Campos da conexão excederam o limite');
 const backend=new URL(endpoint);
 const redirect=new URL(redirectUrl);
 if(backend.protocol!=='https:'||backend.username||backend.password||
  backend.search||backend.hash||!backend.hostname||backend.pathname!=='/')
  throw new Error('Endereço do servidor deve ser HTTPS sem caminho adicional');
 if(redirect.protocol!=='https:'||redirect.username||redirect.password||
  redirect.hash||!redirect.hostname)
  throw new Error('Endereço de autorização HTTPS inválido');
 if(!publicKey||/service.role|service_role|sb_secret_/i.test(publicKey))
  throw new Error('Use somente a chave pública/anon, nunca uma chave secreta de administrador');
 return {endpoint,publicKey,redirectUrl};
}
function createServerConfigStore({filePath,env=process.env}){
 if(typeof filePath!=='string'||!path.isAbsolute(filePath))
  throw new TypeError('Caminho de configuração inválido');
 function load(){
  if(env.STREAMMAX_SUPABASE_URL&&env.STREAMMAX_SUPABASE_PUBLIC_KEY&&env.STREAMMAX_LOGIN_REDIRECT_URL){
   return {value:normalizeServerConfig({
    endpoint:env.STREAMMAX_SUPABASE_URL,
    publicKey:env.STREAMMAX_SUPABASE_PUBLIC_KEY,
    redirectUrl:env.STREAMMAX_LOGIN_REDIRECT_URL
   }),source:'environment'};
  }
  try{
   return {value:normalizeServerConfig(JSON.parse(fs.readFileSync(filePath,'utf8'))),source:'file'};
  }catch(error){
   if(error.code==='ENOENT')return {value:null,source:'missing'};
   return {value:null,source:'invalid'};
  }
 }
 function save(input){
  if(env.STREAMMAX_SUPABASE_URL&&env.STREAMMAX_SUPABASE_PUBLIC_KEY&&env.STREAMMAX_LOGIN_REDIRECT_URL)
   throw new Error('Conexão gerenciada pelas variáveis do Windows; edite essas variáveis');
  const value=normalizeServerConfig(input);
  fs.mkdirSync(path.dirname(filePath),{recursive:true});
  const temp=filePath+'.tmp';
  try{
   fs.writeFileSync(temp,JSON.stringify(value),{mode:0o600,encoding:'utf8',flag:'w'});
   fs.renameSync(temp,filePath);
  }catch(error){try{fs.rmSync(temp,{force:true});}catch{}throw error;}
  return {saved:true};
 }
 return {load,save};
}
module.exports={normalizeServerConfig,createServerConfigStore};
