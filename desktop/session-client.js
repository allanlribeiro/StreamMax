'use strict';
// Validates and refreshes a saved Supabase session entirely in Electron main.
// No token is returned to the renderer. Network failures never erase stored credentials.
function createSessionClient({endpoint,publicKey,store,transport=fetch}) {
  if(!store||typeof store.load!=='function'||typeof store.save!=='function'||typeof store.clear!=='function')
    throw new TypeError('Protected session store required');
  const base = new URL(endpoint);
  if(base.protocol!=='https:'||base.username||base.password||base.search||base.hash)
    throw new TypeError('HTTPS authentication origin required');
  if(typeof publicKey!=='string'||!publicKey)throw new TypeError('Public key required');
  let inFlight=null;
  let sessionGeneration=0;
  const root=base.href.endsWith('/')?base.href:base.href+'/';
  async function call(route,{method='GET',body,token}={}) {
    const headers={'apikey':publicKey,'accept':'application/json'};
    if(body!==undefined)headers['content-type']='application/json';
    if(token)headers.authorization='Bearer '+token;
    return transport(new URL(route,root).toString(),{
      method,headers,...(body===undefined?{}:{body:JSON.stringify(body)}),
      signal:AbortSignal.timeout(12000)
    });
  }
  async function validate(token) {
    const response=await call('auth/v1/user',{token});
    if(response.status===401||response.status===403)return {valid:false,invalid:true};
    if(!response.ok)throw new Error('Falha temporária na validação da sessão');
    const user=await response.json();
    return {valid:!!(user&&typeof user.id==='string'&&typeof user.email==='string'&&user.email),invalid:false};
  }
  async function restoreOnce() {
    const saved=await store.load();
    if(!saved)return {authenticated:false,reason:'no_saved_session'};
    try {
      const first=await validate(saved.access_token);
      if(first.valid)return {authenticated:true};
      if(!first.invalid)return {authenticated:false,reason:'remote_session_invalid'};
      const refreshed=await call('auth/v1/token?grant_type=refresh_token',{
        method:'POST',body:{refresh_token:saved.refresh_token}
      });
      if(refreshed.status===400||refreshed.status===401||refreshed.status===403){
        await store.clear();
        return {authenticated:false,reason:'invalid_session'};
      }
      if(!refreshed.ok)throw new Error('Falha temporária na renovação da sessão');
      const body=await refreshed.json();
      const access_token=body.access_token;
      const refresh_token=body.refresh_token;
      if(typeof access_token!=='string'||!access_token||typeof refresh_token!=='string'||!refresh_token)
        throw new Error('Resposta de renovação inválida');
      // Rotate refresh token before validating the new access token.
      await store.save({access_token,refresh_token});
      const final=await validate(access_token);
      if(final.valid)return {authenticated:true,refreshed:true};
      return {authenticated:false,reason:'remote_session_invalid'};
    }catch{
      return {authenticated:false,reason:'validation_unavailable',hasSavedSession:true};
    }
  }
  return {
    async restore(){
      if(!inFlight){
        const generation=sessionGeneration;
        inFlight=restoreOnce().then(result=>generation===sessionGeneration?result:{authenticated:false,reason:'signed_out'})
          .finally(()=>{inFlight=null;});
      }
      return inFlight;
    },
    async signOut(){
      sessionGeneration++;
      const running=inFlight;
      if(running)await running.catch(()=>{});
      await store.clear();
      return {authenticated:false};
    }
  };
}
module.exports={createSessionClient};
