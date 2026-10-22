'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const os=require('node:os');
const path=require('node:path');
const {normalizeServerConfig,createServerConfigStore}=require('../server-config');

const publicSettings={
 endpoint:'https://backend.example/',
 publicKey:'sb_publishable_public-example',
 redirectUrl:'https://login.example.org/authorize'
};
test('accepts only public HTTPS backend settings',()=>{
 assert.deepEqual(normalizeServerConfig(publicSettings),{
  endpoint:'https://backend.example',publicKey:publicSettings.publicKey,
  redirectUrl:publicSettings.redirectUrl
 });
 assert.throws(()=>normalizeServerConfig({...publicSettings,endpoint:'http://backend.example'}),/HTTPS/);
 assert.throws(()=>normalizeServerConfig({...publicSettings,endpoint:'https://backend.example/private'}),/HTTPS/);
 assert.throws(()=>normalizeServerConfig({...publicSettings,redirectUrl:'http://auth.example'}),/HTTPS/);
 assert.throws(()=>normalizeServerConfig({...publicSettings,publicKey:'sb_secret_abc123'}),/chave secreta/);
 assert.throws(()=>normalizeServerConfig({...publicSettings,publicKey:'service_role_abc'}),/chave secreta/);
});
test('first-run configuration is persisted without exposing tokens and can reload',()=>{
 const dir=fs.mkdtempSync(path.join(os.tmpdir(),'streammax-backend-'));
 try{
  const filename=path.join(dir,'server-config.json');
  const config=createServerConfigStore({filePath:filename,env:{}});
  assert.deepEqual(config.load(),{value:null,source:'missing'});
  assert.deepEqual(config.save(publicSettings),{saved:true});
  assert.deepEqual(config.load(),{
   source:'file',value:normalizeServerConfig(publicSettings)
  });
  const stored=fs.readFileSync(filename,'utf8');
  assert.ok(!stored.includes('access_token'));
  assert.ok(!stored.includes('refresh_token'));
 }finally{fs.rmSync(dir,{recursive:true,force:true});}
});
test('environment-managed configuration remains unchanged by user interface',()=>{
 const dir=fs.mkdtempSync(path.join(os.tmpdir(),'streammax-env-'));
 try{
  const env={STREAMMAX_SUPABASE_URL:publicSettings.endpoint,
   STREAMMAX_SUPABASE_PUBLIC_KEY:publicSettings.publicKey,
   STREAMMAX_LOGIN_REDIRECT_URL:publicSettings.redirectUrl};
  const config=createServerConfigStore({filePath:path.join(dir,'unused.json'),env});
  assert.equal(config.load().source,'environment');
  assert.throws(()=>config.save(publicSettings),/gerenciada/);
 }finally{fs.rmSync(dir,{recursive:true,force:true});}
});

test('an invalid environment config fails closed without crashing startup',()=>{
 const dir=fs.mkdtempSync(path.join(os.tmpdir(),'streammax-invalid-'));
 try{
  const config=createServerConfigStore({
   filePath:path.join(dir,'config.json'),
   env:{STREAMMAX_SUPABASE_URL:'http://insecure.example',
    STREAMMAX_SUPABASE_PUBLIC_KEY:'public',
    STREAMMAX_LOGIN_REDIRECT_URL:'https://example.org'}
  });
  assert.deepEqual(config.load(),{value:null,source:'invalid_environment'});
  assert.throws(()=>config.save(publicSettings),/gerenciada/);
 }finally{fs.rmSync(dir,{recursive:true,force:true});}
});
