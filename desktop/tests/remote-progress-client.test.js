const test=require('node:test');
const assert=require('node:assert/strict');
const {createRemoteProgressClient,normalizeRemoteEntry}=require('../remote-progress-client');
test('reads existing Android watch-progress RPC without changing remote account',async()=>{
 const requests=[];
 const transport=async(url,options)=>{
  requests.push({url,options});
  return {ok:true,json:async()=>[
   {content_id:'tt001',content_type:'movie',video_id:'tt001',position:20000,duration:100000,last_watched:4000},
   {content_id:'ttseries',content_type:'series',video_id:'ttseries:1:1',season:1,episode:1,position:30000,duration:100000,last_watched:1000},
   {content_id:'ttseries',content_type:'series',video_id:'ttseries:1:2',season:1,episode:2,position:54000,duration:100000,last_watched:2000}
  ]};
 };
 const client=createRemoteProgressClient({
  sessionClient:{withAccessToken:fn=>fn('jwt-test')},
  endpoint:'https://example.org',publicKey:'public-anon',transport
 });
 const result=await client.list();
 assert.equal(requests.length,1);
 assert.equal(requests[0].url,'https://example.org/rest/v1/rpc/sync_pull_watch_progress');
 assert.equal(requests[0].options.method,'POST');
 assert.equal(requests[0].options.headers.Authorization,'Bearer jwt-test');
 assert.equal(JSON.parse(requests[0].options.body).p_profile_id,1);
 assert.equal(result.length,2);
 assert.equal(result[1].videoId,'ttseries:1:2');
 assert.equal(result[1].name,null);
 assert.equal(result[1].source,'android-account');
});
test('completed latest episode suppresses earlier in-progress episode',async()=>{
 const client=createRemoteProgressClient({
  sessionClient:{withAccessToken:fn=>fn('test')},
  endpoint:'https://example.org',publicKey:'public',
  transport:async()=>({ok:true,json:async()=>[
   {content_id:'series',content_type:'series',video_id:'series:1:1',season:1,episode:1,position:30000,duration:100000,last_watched:100},
   {content_id:'series',content_type:'series',video_id:'series:1:2',season:1,episode:2,position:96000,duration:100000,last_watched:200}
  ]})
 });
 assert.deepEqual(await client.list(),[]);
});
test('invalid records are discarded and tokens never appear in returned results',async()=>{
 assert.equal(normalizeRemoteEntry({content_id:'x',content_type:'series',video_id:'y',duration:100,position:1,last_watched:10}),null);
 assert.equal(normalizeRemoteEntry({content_id:'x',content_type:'movie',video_id:'x',duration:0,position:1,last_watched:10}),null);
 const client=createRemoteProgressClient({
  sessionClient:{withAccessToken:fn=>fn('private-secret')},endpoint:'https://example.org',publicKey:'public',
  transport:async()=>({ok:true,json:async()=>[
   {content_id:'tt1',content_type:'movie',video_id:'tt1',position:500,duration:1000,last_watched:1234,user_id:'secret-user',progress_key:'tt1'}
  ]})
 });
 const result=await client.list();
 assert.equal(result.length,1);
 assert.equal(JSON.stringify(result).includes('private-secret'),false);
 assert.equal(JSON.stringify(result).includes('secret-user'),false);
});
