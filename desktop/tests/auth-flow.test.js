const test=require('node:test');
const assert=require('node:assert/strict');
const {authTransition,sanitizeChallenge}=require('../auth-flow');
test('device login follows approved state transitions',()=>{
 assert.equal(authTransition('idle','start'),'starting');
 assert.equal(authTransition('starting','challenge'),'pending');
 assert.equal(authTransition('pending','approved'),'authorized');
 assert.equal(authTransition('authorized','signout'),'idle');
});
test('rejects unsupported state transitions',()=>assert.throws(()=>authTransition('idle','approved')));
test('normalizes Android TV login challenge without tokens',()=>{
 const result=sanitizeChallenge({webUrl:'https://example.org/verify',code:'ABCDEF',pollIntervalSeconds:7});
 assert.equal(result.userCode,'ABCDEF');
 assert.equal(result.pollIntervalSeconds,7);
 assert.deepEqual(Object.keys(result).sort(),['pollIntervalSeconds','userCode','verificationUrl']);
});
