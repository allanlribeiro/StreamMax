const test=require('node:test');
const assert=require('node:assert/strict');
const {createCatalogClient,catalogUrl,manifestUrl}=require('../catalog-client');
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
