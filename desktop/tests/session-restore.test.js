const test=require('node:test');
const assert=require('node:assert/strict');
const {createAuthService}=require('../auth-service');
test('restore reports missing session without exposing secrets',async()=>{
 const auth=createAuthService({sessionStore:{load:async()=>null,clear:async()=>{}}});
 assert.deepEqual(await auth.restore(),{authenticated:false,reason:'no_saved_session'});
});
test('restore never treats saved but unvalidated credentials as authenticated',async()=>{
 const auth=createAuthService({sessionStore:{load:async()=>({access_token:'secret',refresh_token:'secret'}),clear:async()=>{}}});
 assert.deepEqual(await auth.restore(),{authenticated:false,reason:'remote_session_validation_required',hasSavedSession:true});
});
test('sign out removes saved session and pending state',async()=>{
 let cleared=false;
 const auth=createAuthService({sessionStore:{load:async()=>null,clear:async()=>{cleared=true;}}});
 assert.deepEqual(await auth.signOut(),{authenticated:false});
 assert.equal(cleared,true);
});
