const test=require('node:test');
const assert=require('node:assert/strict');
const {RPC,rpcBody,callLoginRpc}=require('../auth-rpc');
test('matches Android device and legacy start payload fields',()=>{
 const a=rpcBody(RPC.startDevice,{deviceNonce:'random',redirectBaseUrl:'https://example.org/login',deviceType:'windows'});
 assert.equal(a.p_device_type,'windows');
 const b=rpcBody(RPC.startLegacy,{deviceNonce:'random',redirectBaseUrl:'https://example.org/login'});
 assert.equal(b.p_device_type,undefined);
});
test('matches Android poll and exchange payloads',()=>{
 assert.deepEqual(rpcBody(RPC.poll,{deviceNonce:'random',code:'XYZ'}),{p_code:'XYZ',p_device_nonce:'random'});
 assert.deepEqual(rpcBody(RPC.exchange,{deviceNonce:'random',code:'XYZ'}),{code:'XYZ',device_nonce:'random'});
});
test('requires trusted transport and blocks insecure service origin',async()=>{
 await assert.rejects(()=>callLoginRpc({transport:async()=>{},endpoint:'http://example.org',publicKey:'public',method:RPC.poll,options:{code:'X',deviceNonce:'a'}}));
});

test('preserves service subpath when constructing device login RPC URL',async()=>{
 let called;
 await callLoginRpc({
  transport:async(url)=>{called=url;return {ok:true};},
  endpoint:'https://example.org/backend/',
  publicKey:'public',
  method:RPC.poll,
  options:{code:'X',deviceNonce:'a'}
 });
 assert.equal(called,'https://example.org/backend/rest/v1/rpc/poll_tv_login_session');
});

test('uses the Android edge function for login exchange',async()=>{
 let url;
 await callLoginRpc({
  transport:async u=>{url=u;return {ok:true};},
  endpoint:'https://example.org',
  publicKey:'public',
  method:RPC.exchange,
  options:{code:'ABC',deviceNonce:'nonce'}
 });
 assert.equal(url,'https://example.org/functions/v1/tv-logins-exchange');
});
