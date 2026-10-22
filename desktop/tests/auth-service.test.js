const test = require('node:test');
const assert = require('node:assert/strict');
const {createAuthService} = require('../auth-service');

test('refuses login before the public service is configured', async () => {
  const service = createAuthService({});
  assert.deepEqual(service.status(), {configured:false,pending:false});
  await assert.rejects(service.start(), /não configurado/);
});
test('keeps nonce private and allows cancelling the login attempt', async () => {
  const calls=[];
  const transport=async (url,request)=>{
    calls.push({url,body:JSON.parse(request.body)});
    return {ok:true,json:async()=>[{device_code:'secret-code',user_code:'DISPLAY',verification_uri_complete:'https://example.org/verify'}]};
  };
  const service=createAuthService({endpoint:'https://example.org',publicKey:'public',redirectUrl:'https://example.org/login',transport});
  const challenge=await service.start();
  assert.deepEqual(challenge,{userCode:'DISPLAY',verificationUrl:'https://example.org/verify',legacy:false,pollIntervalSeconds:5});
  assert.equal(service.status().pending,true);
  assert.equal(Object.hasOwn(challenge,'deviceNonce'),false);
  assert.ok(calls[0].body.p_device_nonce);
  assert.equal(service.cancel().pending,false);
  await assert.rejects(service.poll(),/Nenhum login pendente/);
});

test('falls back to legacy TV login only for an unavailable device RPC',async()=>{
 const calls=[];
 const transport=async(url)=>{
   calls.push(url);
   if(url.endsWith('/start_device_login_session'))return {ok:false,status:404,text:async()=> 'Could not find the function start_device_login_session'};
   return {ok:true,json:async()=>[{code:'LEGACY',web_url:'https://example.org/legacy'}]};
 };
 const service=createAuthService({endpoint:'https://example.org',publicKey:'public',redirectUrl:'https://example.org/login',transport});
 const challenge=await service.start();
 assert.deepEqual(challenge,{userCode:'LEGACY',verificationUrl:'https://example.org/legacy',legacy:true,pollIntervalSeconds:5});
 assert.equal(calls.length,2);
});

test('approved device is not treated as signed in without secure token exchange',async()=>{
 let count=0;
 const transport=async()=>{
  count++;
  return {ok:true,json:async()=>count===1
   ? [{device_code:'device',user_code:'ABCD',verification_uri_complete:'https://example.org/login'}]
   : [{status:'approved'}]};
 };
 const service=createAuthService({endpoint:'https://example.org',publicKey:'public',redirectUrl:'https://example.org/login',transport});
 await service.start();
 const result=await service.poll();
 assert.deepEqual(result,{status:'awaiting_secure_session',authenticated:false});
});
