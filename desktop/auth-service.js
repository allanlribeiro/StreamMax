'use strict';
const {randomBytes}=require('node:crypto');
const {RPC,callLoginRpc}=require('./auth-rpc');
function createAuthService({endpoint,publicKey,redirectUrl,transport=fetch,sessionStore=null}){
 let pending=null;
 const configured=Boolean(endpoint&&publicKey&&redirectUrl);
 async function request(method,options){
  const response=await callLoginRpc({transport,endpoint,publicKey,method,options});
  if(!response.ok) {
    const detail=await response.text().catch(()=>'');
    const missingFunction=response.status===404 && /could not find the function/i.test(detail) && /start_device_login_session/i.test(detail);
    const error=new Error('Falha no serviço de autenticação ('+response.status+')');
    error.legacyFallback=missingFunction;
    throw error;
  }
  const body=await response.json();
  if(method===RPC.exchange){
    if(!body||Array.isArray(body)||typeof body!=='object')throw new Error('Resposta de troca de sessão inválida');
    return body;
  }
  if(!Array.isArray(body)||!body[0])throw new Error('Resposta de autenticação vazia');
  return body[0];
 }
 return {
  status(){return {configured,pending:!!pending};},
  async start(){
   if(!configured)throw new Error('Servidor de autenticação ainda não configurado para Windows');
   const nonce=randomBytes(32).toString('hex');
   let result, legacy=false;
   try {
     result=await request(RPC.startDevice,{deviceNonce:nonce,redirectBaseUrl:redirectUrl,deviceType:'windows',deviceName:'StreamMax Windows'});
   } catch(error) {
     if(!error.legacyFallback)throw error;
     result=await request(RPC.startLegacy,{deviceNonce:nonce,redirectBaseUrl:redirectUrl,deviceName:'StreamMax Windows'});
     legacy=true;
   }
   const code=String(result.device_code||result.deviceCode||result.code||'');
   const userCode=String(result.user_code||result.userCode||result.code||'');
   const verificationUrl=String(result.verification_uri_complete||result.verificationUriComplete||result.web_url||result.webUrl||'');
   if(!code||!userCode||new URL(verificationUrl).protocol!=='https:')throw new Error('Resposta de login inválida');
   pending={nonce,code};
   return {userCode,verificationUrl,legacy};
  },
  async poll(){
   if(!pending)throw new Error('Nenhum login pendente');
   const result=await request(RPC.poll,{deviceNonce:pending.nonce,code:pending.code});
   const status=String(result.status||'').toLowerCase();
   if(status==='expired'||status==='denied'||status==='rejected')pending=null;
   // Android exchanges approved codes for access/refresh tokens. This Windows
   // preview must not claim to be signed in before secure storage is available.
   if(status==='approved'||status==='authorized'||status==='completed'){
     if(!sessionStore)return {status:'awaiting_secure_session',authenticated:false};
     const exchanged=await request(RPC.exchange,{deviceNonce:pending.nonce,code:pending.code});
     const access_token=exchanged.access_token||exchanged.accessToken;
     const refresh_token=exchanged.refresh_token||exchanged.refreshToken;
     if(!access_token||!refresh_token)throw new Error('Invalid token exchange response');
     await sessionStore.save({access_token,refresh_token});
     pending=null;
     return {status:'authenticated',authenticated:true};
   }
   return {status,authenticated:false};
  },
  cancel(){pending=null;return {pending:false};}
 };
}
module.exports={createAuthService};
