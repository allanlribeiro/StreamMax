const test=require('node:test');
const assert=require('node:assert/strict');
const {createAuthService}=require('../auth-service');
test('approved device exchanges tokens and stores them before reporting authentication',async()=>{
 let requests=0;
 let saved=null;
 const transport=async(url)=>{
  requests++;
  if(url.includes('start_device_login_session'))return {ok:true,json:async()=>[{device_code:'device',user_code:'ABC',verification_uri_complete:'https://example.org/verify'}]};
  if(url.includes('poll_tv_login_session'))return {ok:true,json:async()=>[{status:'approved'}]};
  if(url.includes('tv-logins-exchange'))return {ok:true,json:async()=>({access_token:'access',refresh_token:'refresh'})};
  throw new Error('Unexpected endpoint');
 };
 const service=createAuthService({endpoint:'https://example.org',publicKey:'public',redirectUrl:'https://example.org/verify',transport,sessionStore:{save:async x=>{saved=x;}}});
 await service.start();
 assert.deepEqual(await service.poll(),{status:'authenticated',authenticated:true});
 assert.deepEqual(saved,{access_token:'access',refresh_token:'refresh'});
 assert.equal(requests,3);
});
test('never reports authenticated when protected storage fails',async()=>{
 const transport=async url=>({ok:true,json:async()=>url.includes('start_device_login_session')?[{device_code:'code',user_code:'CODE',verification_uri_complete:'https://example.org/verify'}]:url.includes('poll_tv_login_session')?[{status:'approved'}]:{access_token:'x',refresh_token:'y'}});
 const service=createAuthService({endpoint:'https://example.org',publicKey:'public',redirectUrl:'https://example.org/login',transport,sessionStore:{save:async()=>{throw new Error('storage unavailable');}}});
 await service.start();
 await assert.rejects(service.poll(),/storage unavailable/);
});
