'use strict';
// Desktop-side login state machine aligned to Android AuthManager:
// startDeviceLoginSession / startTvLoginSession / pollTvLoginSession / exchangeTvLoginSession.
// Transport and protected token storage MUST be injected separately.
const allowed=['idle','starting','pending','authorized','expired','failed'];
function authTransition(state,event) {
 if(!allowed.includes(state))throw new TypeError('Invalid auth state');
 const next={
  idle:{start:'starting'},
  starting:{challenge:'pending',error:'failed',cancel:'idle'},
  pending:{approved:'authorized',expired:'expired',error:'failed',cancel:'idle'},
  authorized:{signout:'idle'},
  expired:{retry:'starting',cancel:'idle'},
  failed:{retry:'starting',cancel:'idle'}
 }[state][event];
 if(!next)throw new TypeError('Invalid auth transition');
 return next;
}
function sanitizeChallenge(challenge){
 if(!challenge||typeof challenge!=='object')throw new TypeError('Missing login challenge');
 const verificationUrl=String(challenge.verificationUriComplete||challenge.webUrl||'');
 const u=new URL(verificationUrl);
 if(u.protocol!=='https:'||u.username||u.password)throw new TypeError('Invalid verification URL');
 const code=String(challenge.userCode||challenge.code||'').trim();
 const interval=Number(challenge.pollIntervalSeconds||5);
 if(!code||!Number.isFinite(interval)||interval<1)throw new TypeError('Invalid challenge');
 return {verificationUrl:u.toString(),userCode:code,pollIntervalSeconds:interval};
}
module.exports={authTransition,sanitizeChallenge};
