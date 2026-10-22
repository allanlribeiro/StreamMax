'use strict';
// Implements the server-side request contract used by Android AuthManager.
// Caller injects an HTTPS POST transport and the configured public endpoint.
// Never persist a token or device nonce in renderer/localStorage/logs.
const RPC = Object.freeze({
 startDevice:'start_device_login_session',
 startLegacy:'start_tv_login_session',
 poll:'poll_tv_login_session',
 exchange:'exchange_tv_login_session'
});
function rpcBody(method, options) {
 if(!Object.values(RPC).includes(method))throw new TypeError('Unsupported login RPC');
 if(!options || typeof options!=='object')throw new TypeError('Options required');
 const nonce=String(options.deviceNonce||'');
 if(!nonce)throw new TypeError('Device nonce required');
 if(method===RPC.startDevice||method===RPC.startLegacy) {
  const redirect=String(options.redirectBaseUrl||'');
  if(new URL(redirect).protocol!=='https:')throw new TypeError('HTTPS redirect required');
  const payload={p_device_nonce:nonce,p_redirect_base_url:redirect};
  if(method===RPC.startDevice)payload.p_device_type=String(options.deviceType||'windows');
  if(options.deviceName)payload.p_device_name=String(options.deviceName);
  return payload;
 }
 const code=String(options.code||'');
 if(!code)throw new TypeError('Login code required');
 return method===RPC.poll?{p_code:code,p_device_nonce:nonce}:{code,device_nonce:nonce};
}
async function callLoginRpc({transport,endpoint,publicKey,method,options}){
 if(typeof transport!=='function')throw new TypeError('A transport is required');
 const base=new URL(endpoint);
 if(base.protocol!=='https:'||base.username||base.password||base.search||base.hash)throw new TypeError('Invalid service endpoint');
 if(typeof publicKey!=='string'||!publicKey)throw new TypeError('Public client key required');
 // Network call lives in the trusted main process, not the renderer.
 const route=method===RPC.exchange?'functions/v1/tv-logins-exchange':'rest/v1/rpc/'+method;
 const url=new URL(route,base.href.endsWith('/')?base.href:base.href+'/');
 if(!Object.values(RPC).includes(method))throw new TypeError('Invalid RPC');
 return transport(url.toString(),{method:'POST',headers:{'content-type':'application/json',apikey:publicKey},body:JSON.stringify(rpcBody(method,options))});
}
module.exports={RPC,rpcBody,callLoginRpc};
