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
  assert.deepEqual(challenge,{userCode:'DISPLAY',verificationUrl:'https://example.org/verify'});
  assert.equal(service.status().pending,true);
  assert.equal(Object.hasOwn(challenge,'deviceNonce'),false);
  assert.ok(calls[0].body.p_device_nonce);
  assert.equal(service.cancel().pending,false);
  await assert.rejects(service.poll(),/Nenhum login pendente/);
});
