const test=require('node:test');
const assert=require('node:assert/strict');
const {createCatalogClient,catalogUrl,metaUrl,manifestUrl}=require('../catalog-client');
test('builds catalog routes and pagination like Android',()=>{
 assert.equal(catalogUrl('https://addon.example/base','movie','popular',0),
   'https://addon.example/base/catalog/movie/popular.json');
 assert.equal(catalogUrl('https://addon.example/base?key=abc','series','trending',100),
   'https://addon.example/base/catalog/series/trending/skip=100.json?key=abc');
 assert.equal(catalogUrl('https://addon.example/base','movie','popular',100,{genre:'Sci Fi'}),
   'https://addon.example/base/catalog/movie/popular/genre=Sci%20Fi&skip=100.json');
 assert.equal(manifestUrl('https://addon.example/base/manifest.json?key=abc'),
   'https://addon.example/base/manifest.json?key=abc');
});
test('reads account addon order and real catalog items, never forwards account bearer to addons',async()=>{
 const accountCalls=[],addonCalls=[];
 const sessionClient={withAccessToken:fn=>fn('private-user-token')};
 const transport=async(url,options)=>{
  accountCalls.push({url,options});
  if(url.includes('get_sync_owner'))return {ok:true,json:async()=>('owner-id')};
  return {ok:true,json:async()=>[
   {url:'https://addon.example/one',name:'Meus filmes',sort_order:0,enabled:true},
   {url:'https://addon.example/two',name:'Desligado',sort_order:1,enabled:false}
  ]};
 };
 const addonTransport=async(url,options)=>{
   addonCalls.push({url,options});
   if(url.includes('manifest.json'))return {ok:true,json:async()=>({
     id:'movies-addon',name:'My catalog',catalogs:[{id:'popular',name:'Populares',type:'movie',extraSupported:['skip']}]
   })};
   return {ok:true,json:async()=>({metas:[{id:'tt123',type:'movie',name:'Filme real',poster:'https://images.example/film.jpg'}]})};
 };
 const client=createCatalogClient({endpoint:'https://backend.example',publicKey:'public',sessionClient,transport,addonTransport});
 const list=await client.reload();
 assert.equal(list.addons.length,2);
 assert.equal(list.addons[0].catalogCount,1);
 assert.equal(list.catalogs[0].catalogName,'Populares');
 assert.equal(Object.hasOwn(list.addons[0],'url'),false);
 const result=await client.page(list.catalogs[0].key,0);
 assert.equal(result.items[0].name,'Filme real');
 assert.equal(result.items[0].poster,'https://images.example/film.jpg');
 assert.equal(result.hasMore,true);
 assert.ok(accountCalls.every(x=>x.options.headers.Authorization==='Bearer private-user-token'));
 assert.ok(addonCalls.every(x=>!('Authorization' in x.options.headers)&&!('apikey' in x.options.headers)));
});
test('rejects private hosts and unknown catalog requests',async()=>{
 assert.throws(()=>manifestUrl('http://addon.example'));
 assert.throws(()=>manifestUrl('https://127.0.0.1/addon'));
 const client=createCatalogClient({endpoint:'https://backend.example',publicKey:'public',sessionClient:{withAccessToken:fn=>fn('private')}});
 await assert.rejects(client.page('unknown',0),/Unknown catalog/);
});

test('removes prior profile catalogs when session signs out',async()=>{
 const client=createCatalogClient({
  endpoint:'https://backend.example',publicKey:'public',
  sessionClient:{withAccessToken:fn=>fn('private')},
  transport:async url=>({ok:true,json:async()=>url.includes('get_sync_owner')?'owner-id':[
    {url:'https://addon.example/base',name:'Addon',enabled:true,sort_order:0}
  ]}),
  addonTransport:async()=>({ok:true,json:async()=>({id:'addon',name:'Addon',catalogs:[{id:'popular',type:'movie'}]})})
 });
 const list=await client.reload();
 assert.equal(list.catalogs.length,1);
 client.clear();
 await assert.rejects(client.page(list.catalogs[0].key),/Unknown catalog/);
});

test('enables only manifest-advertised stream providers and clears them on logout',async()=>{
 const c=createCatalogClient({
  endpoint:'https://backend.example',publicKey:'public',
  sessionClient:{withAccessToken:fn=>fn('private')},
  transport:async url=>({ok:true,json:async()=>url.includes('get_sync_owner')?'owner-id':[
   {url:'https://video.example/addon',name:'Vídeos',enabled:true,sort_order:0},
   {url:'https://metadata.example/addon',name:'Metadados',enabled:true,sort_order:1}
  ]}),
  addonTransport:async url=>({ok:true,json:async()=>({
   id:'addon',name:'Addon',resources:url.includes('video.example')?['catalog','stream']:['catalog','meta'],
   catalogs:[{id:'popular',type:'movie'}]
  })})
 });
 await c.reload();
 assert.equal(c.getSourceProviders().length,1);
 assert.equal(c.getSourceProviders()[0].name,'Vídeos');
 c.clear();
 assert.deepEqual(c.getSourceProviders(),[]);
});

test('fetches series metadata and true episode IDs without sharing Supabase bearer',async()=>{
 const addonCalls=[];
 const c=createCatalogClient({
  endpoint:'https://backend.example',publicKey:'public',
  sessionClient:{withAccessToken:fn=>fn('private')},
  transport:async url=>({ok:true,json:async()=>url.includes('get_sync_owner')?'owner-id':[
   {url:'https://series.example/secret?key=addon-key',enabled:true,name:'Séries'}
  ]}),
  addonTransport:async(url,options)=>{
   addonCalls.push({url,options});
   if(url.includes('manifest.json'))return {ok:true,json:async()=>({id:'series-addon',name:'Séries',catalogs:[{id:'featured',type:'series'}]})};
   return {ok:true,json:async()=>({meta:{
    id:'tt001',type:'series',name:'Minha série',genres:['Drama'],videos:[
     {id:'tt001:1:1',season:1,episode:1,title:'Piloto'},
     {id:'tt001:2:3',season:2,episode:3,title:'Continuação',thumbnail:'https://img.example/3.jpg'}
    ]
   }})};
  }
 });
 const rows=await c.reload();
 const detail=await c.meta(rows.catalogs[0].key,'series','tt001');
 assert.deepEqual(detail.videos.map(ep=>ep.id),['tt001:1:1','tt001:2:3']);
 assert.equal(detail.videos[1].season,2);
 assert.equal(detail.videos[1].episode,3);
 assert.equal(addonCalls.at(-1).url,'https://series.example/secret/meta/series/tt001.json?key=addon-key');
 assert.ok(addonCalls.every(v=>!v.options.headers.apikey&&!v.options.headers.Authorization));
});
test('accepts stream-only addons that omit catalog declarations',async()=>{
 const c=createCatalogClient({
  endpoint:'https://backend.example',publicKey:'public',
  sessionClient:{withAccessToken:fn=>fn('private')},
  transport:async url=>({ok:true,json:async()=>url.includes('get_sync_owner')?'owner-id':[
   {url:'https://streams.example/base',enabled:true,name:'Fontes'}
  ]}),
  addonTransport:async()=>({ok:true,json:async()=>({id:'sources',name:'Fontes',resources:['stream']})})
 });
 const rows=await c.reload();
 assert.equal(rows.catalogs.length,0);
 assert.equal(c.getSourceProviders().length,1);
});
test('enforces catalog origin for metadata and encodes episode lookup safely',async()=>{
 assert.equal(metaUrl('https://addon.example/private?token=addon','series','tt123:1:2'),
  'https://addon.example/private/meta/series/tt123%3A1%3A2.json?token=addon');
 const c=createCatalogClient({endpoint:'https://backend.example',publicKey:'public',sessionClient:{withAccessToken:fn=>fn('private')}});
 await assert.rejects(c.meta('unknown','series','tt123'),/Unknown catalog/);
});

test('fetches manifests concurrently but preserves saved addon ordering',async()=>{
 let unblockSlow;
 const slow=new Promise(resolve=>{unblockSlow=resolve;});
 const requests=[];
 const c=createCatalogClient({
  endpoint:'https://backend.example',publicKey:'public',
  sessionClient:{withAccessToken:fn=>fn('private')},
  transport:async url=>({ok:true,json:async()=>url.includes('get_sync_owner')?'owner-id':[
   {url:'https://slow.example/path',name:'Primeiro',enabled:true,sort_order:0},
   {url:'https://fast.example/path',name:'Segundo',enabled:true,sort_order:1}
  ]}),
  addonTransport:async url=>{
   requests.push(url);
   if(url.includes('slow.example'))await slow;
   return {ok:true,json:async()=>({
    id:url.includes('slow.example')?'slow':'fast',
    name:'Addon',
    resources:['catalog'],
    catalogs:[{id:'popular',name:'Popular',type:'movie'}]
   })};
  }
 });
 const loading=c.reload();
 for(let i=0;i<20&&requests.length<2;i++)await new Promise(resolve=>setImmediate(resolve));
 assert.equal(requests.length,2,'both manifests should start without waiting for slow host');
 unblockSlow();
 const results=await loading;
 assert.deepEqual(results.addons.map(v=>v.name),['Primeiro','Segundo']);
 assert.deepEqual(results.catalogs.map(v=>v.addonId),['slow','fast']);
});
