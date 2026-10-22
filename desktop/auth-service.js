'use strict';
const {randomBytes}=require('node:crypto');
const {RPC,callLoginRpc}=require('./auth-rpc');
function createAuthService({endpoint,publicKey,redirectUrl,transport=fetch}){
 let pending=null;
 const configured=Boolean(endpoint&&publicKey&&redirectUrl);
 async function request(method,options){
  const response=await callLoginRpc({transport,endpoint,publicKey,method,options});
  if(!response.ok)throw new Error('Falha no serviço de autenticação ('+response.status+')');
  const body=await response.json();
  if(!Array.isArray(body)||!body[0])throw new Error('Resposta de autenticação vazia');
  return body[0];
 }
 return {
  status(){return {configured,pending:!!pending};},
  async start(){
   if(!configured)throw new Error('Servidor de autenticação ainda não configurado para Windows');
   const nonce=randomBytes(32).toString('hex');
   const result=await request(RPC.startDevice,{deviceNonce:nonce,redirectBaseUrl:redirectUrl,deviceType:'windows',deviceName:'StreamMax Windows'});
   const code=String(result.device_code||result.deviceCode||'');
   const userCode=String(result.user_code||result.userCode||'');
   const verificationUrl=String(result.verification_uri_complete||result.verificationUriComplete||'');
   if(!code||!userCode||new URL(verificationUrl).protocol!=='https:')throw new Error('Resposta de login inválida');
   pending={nonce,code};
   return {userCode,verificationUrl};
  },
  async poll(){
   if(!pending)throw new Error('Nenhum login pendente');
   const result=await request(RPC.poll,{deviceNonce:pending.nonce,code:pending.code});
   const status=String(result.status||'').toLowerCase();
   if(status==='expired'||status==='denied'||status==='rejected')pending=null;
   // No exchange of credentials until Windows protected token storage is integrated.
   return {status};
  },
  cancel(){pending=null;return {pending:false};}
 };
}
module.exports={createAuthService};
