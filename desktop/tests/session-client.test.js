const test=require('node:test');
const assert=require('node:assert/strict');
const {createSessionClient}=require('../session-client');
const config={endpoint:'https://example.org',publicKey:'demo-public'};
test('valid session is restored from user endpoint without exposing tokens',async()=>{
 let count=0;
 const store={load:async()=>({access_token:'old',refresh_token:'refresh'}),save:async()=>{throw new Error('Unexpected save');},clear:async()=>{}};
 const transport=async (url,options)=>{
   assert.equal(url,'https://example.org/auth/v1/user');
   assert.equal(options.headers.authorization,'Bearer old');
   count++;
   return {ok:true,status:200,json:async()=>({id:'user',email:'example@example.org'})};
 };
 const client=createSessionClient({...config,store,transport});
 assert.deepEqual(await client.restore(),{authenticated:true});
 assert.equal(count,1);
});
test('expired access token is refreshed, persisted, then revalidated',async()=>{
 const requests=[];
 let saved;
 const store={load:async()=>({access_token:'old',refresh_token:'refresh-old'}),save:async session=>{saved=session;},clear:async()=>{}};
 const transport=async(url,options)=>{
  requests.push({url,options});
  if(requests.length===1)return {ok:false,status:401};
  if(requests.length===2)return {ok:true,status:200,json:async()=>({access_token:'new',refresh_token:'refresh-new'})};
  return {ok:true,status:200,json:async()=>({id:'user',email:'a@example.org'})};
 };
 const client=createSessionClient({...config,store,transport});
 assert.deepEqual(await client.restore(),{authenticated:true,refreshed:true});
 assert.deepEqual(saved,{access_token:'new',refresh_token:'refresh-new'});
 assert.equal(requests[1].url,'https://example.org/auth/v1/token?grant_type=refresh_token');
 assert.equal(requests[2].options.headers.authorization,'Bearer new');
});
test('invalid refresh clears unusable credentials, transient errors preserve them',async()=>{
 let clearCount=0;
 const store={load:async()=>({access_token:'old',refresh_token:'old'}),save:async()=>{},clear:async()=>{clearCount++;}};
 const expired=async(url)=>({ok:false,status:url.includes('/user')?401:400});
 assert.deepEqual(await createSessionClient({...config,store,transport:expired}).restore(),{authenticated:false,reason:'invalid_session'});
 assert.equal(clearCount,1);
 const offline=async()=>{throw new Error('Network offline');};
 assert.deepEqual(await createSessionClient({...config,store,transport:offline}).restore(),{authenticated:false,reason:'validation_unavailable',hasSavedSession:true});
 assert.equal(clearCount,1);
});
test('missing local session never contacts the server',async()=>{
 const store={load:async()=>null,save:async()=>{},clear:async()=>{}};
 const transport=async()=>{throw new Error('Unexpected network');};
 assert.deepEqual(await createSessionClient({...config,store,transport}).restore(),{authenticated:false,reason:'no_saved_session'});
});

test('sign-out waits for in-flight restore and removes re-created token data',async()=>{
 let resumeRefresh;
 const wait=new Promise(resolve=>{resumeRefresh=resolve;});
 let session={access_token:'expired',refresh_token:'old'};
 const store={
   load:async()=>session,
   save:async value=>{session=value;},
   clear:async()=>{session=null;}
 };
 const transport=async url=>{
   if(url.includes('/user'))return {ok:false,status:401};
   await wait;
   return {ok:true,status:200,json:async()=>({access_token:'new',refresh_token:'new-refresh'})};
 };
 const client=createSessionClient({...config,store,transport});
 const restoring=client.restore();
 await Promise.resolve();
 const signingOut=client.signOut();
 resumeRefresh();
 assert.deepEqual(await restoring,{authenticated:false,reason:'signed_out'});
 assert.deepEqual(await signingOut,{authenticated:false});
 assert.equal(session,null);
});
