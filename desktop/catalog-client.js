'use strict';
// Read-only bridge for the Android AddonSyncService + CatalogRepositoryImpl contracts.
// Supabase bearer tokens must NEVER be sent to third-party Stremio addons.
const {catalogRequest}=require('./catalog-contract');

function safeAddonUrl(raw){
  const url=new URL(raw);
  const host=url.hostname.toLowerCase();
  if(url.protocol!=='https:'||url.username||url.password||!host||
    host==='localhost'||host.endsWith('.localhost')||host.endsWith('.local')||
    host==='127.0.0.1'||host==='0.0.0.0'||host==='::1'||
    /^10\./.test(host)||/^192\.168\./.test(host)||/^172\.(1[6-9]|2\d|3[01])\./.test(host)||
    /^169\.254\./.test(host)||/^fc[0-9a-f]{2}:/.test(host))
    throw new Error('Unsupported addon host');
  return url;
}
function catalogUrl(base,type,id,skip=0,extraArgs={}){
  const parsed=safeAddonUrl(base);
  const basePath=parsed.pathname.replace(/\/$/,'').replace(/\/manifest\.json$/i,'');
  const parts=basePath+'/catalog/'+encodeURIComponent(type)+'/'+encodeURIComponent(id);
  const extra={...extraArgs};
  if(Object.keys(extra).length===0){
    parsed.pathname=parts+(skip>0?'/skip='+skip:'')+'.json';
  }else{
    if(skip>0&&!Object.prototype.hasOwnProperty.call(extra,'skip'))extra.skip=String(skip);
    const keys=Object.entries(extra).map(([k,v])=>encodeURIComponent(k)+'='+encodeURIComponent(v)).join('&');
    parsed.pathname=parts+'/'+keys+'.json';
  }
  return parsed.toString();
}
function manifestUrl(raw){
 const url=safeAddonUrl(raw);
 url.pathname=url.pathname.replace(/\/$/,'').replace(/\/manifest\.json$/i,'')+'/manifest.json';
 return url.toString();
}
function createCatalogClient({sessionClient,endpoint,publicKey,profileId=1,transport=fetch,addonTransport=fetch}){
 if(!sessionClient||typeof sessionClient.withAccessToken!=='function')throw new TypeError('Session client required');
 if(!Number.isInteger(profileId)||profileId<1)throw new TypeError('Invalid profile');
 const root=new URL(endpoint);
 if(root.protocol!=='https:'||root.username||root.password||root.search||root.hash)throw new TypeError('HTTPS backend required');
 const base=root.href.replace(/\/$/,'')+'/';
 const apiKey=String(publicKey||'');
 if(!apiKey)throw new TypeError('Public client key required');
 let registered=[];
 let catalogs=new Map();
 async function readJson(url,opts={},client=transport){
  const response=await client(url,{...opts,redirect:'error',signal:AbortSignal.timeout(14000)});
  if(!response.ok)throw new Error('Catalog service returned HTTP '+response.status);
  return response.json();
 }
 async function reload(){
  // Fetch owner ID and user-specific add-ons with same RLS-backed database as Android.
  const addons=await sessionClient.withAccessToken(async token=>{
   const headers={apikey:apiKey,Authorization:'Bearer '+token,Accept:'application/json'};
   const owner=await readJson(new URL('rest/v1/rpc/get_sync_owner',base).toString(),
    {method:'POST',headers:{...headers,'Content-Type':'application/json'},body:'{}'});
   if(typeof owner!=='string'||!owner.trim()||owner.length>128)throw new Error('Invalid sync owner');
   const params=new URLSearchParams({select:'url,name,enabled,sort_order',user_id:'eq.'+owner,profile_id:'eq.'+String(profileId),order:'sort_order.asc'});
   const rows=await readJson(new URL('rest/v1/addons?'+params,base).toString(),{headers});
   if(!Array.isArray(rows))throw new Error('Invalid addon account data');
   return rows;
  });
  const nextCatalogs=new Map();
  const nextRegistered=[];
  for(let i=0;i<addons.length;i++){
    const item=addons[i];
    if(!item||typeof item.url!=='string')continue;
    const identifier='addon-'+i;
    const enabled=item.enabled!==false;
    const entry={id:identifier,name:String(item.name||''),enabled,catalogCount:0};
    if(enabled){
      try{
        const url=manifestUrl(item.url);
        // No Authorization or Supabase API key on addon requests.
        const m=await readJson(url,{headers:{Accept:'application/json'}},addonTransport);
        if(!m||!Array.isArray(m.catalogs))throw new Error('Invalid addon manifest');
        entry.name=entry.name||String(m.name||'');
        for(let j=0;j<m.catalogs.length;j++){
          const desc=m.catalogs[j];
          if(!desc||typeof desc.id!=='string'||typeof desc.type!=='string')continue;
          if(desc.showInHome===false)continue;
          const key=identifier+'-catalog-'+j;
          const caption=String(desc.name||desc.id);
          nextCatalogs.set(key,{addonId:String(m.id||identifier),addonName:entry.name,baseUrl:item.url,catalogId:desc.id,catalogName:caption,type:desc.type,supportsSkip:Array.isArray(desc.extraSupported)&&desc.extraSupported.includes('skip')||Array.isArray(desc.extra)&&desc.extra.some(x=>x&&x.name==='skip')});
        }
        entry.catalogCount=Array.from(nextCatalogs.keys()).filter(k=>k.startsWith(identifier+'-')).length;
      }catch{ entry.error='manifest_unavailable'; }
    }
    nextRegistered.push(entry);
  }
  registered=nextRegistered;
  catalogs=nextCatalogs;
  return {addons:registered.map(x=>({...x})),catalogs:Array.from(catalogs,( [key,v] )=>({
    key,addonId:v.addonId,addonName:v.addonName,catalogId:v.catalogId,catalogName:v.catalogName,type:v.type
  }))};
 }
 async function page(key,skip=0,extraArgs={}){
  const item=catalogs.get(key);
  if(!item)throw new Error('Unknown catalog');
  const args=catalogRequest({
   addonBaseUrl:item.baseUrl,addonId:item.addonId,addonName:item.addonName,
   catalogId:item.catalogId,catalogName:item.catalogName,type:item.type,
   skip,supportsSkip:item.supportsSkip,extraArgs
  });
  const url=catalogUrl(args.addonBaseUrl,args.type,args.catalogId,args.skip,args.extraArgs);
  const body=await readJson(url,{headers:{Accept:'application/json'}},addonTransport);
  if(!body||!Array.isArray(body.metas))throw new Error('Invalid catalog response');
  const seen=new Set();
  const items=body.metas.filter(x=>x&&typeof x.id==='string'&&typeof x.name==='string')
   .filter(x=>{const k=String(x.type||args.type)+':'+x.id;if(seen.has(k))return false;seen.add(k);return true;})
   .map(x=>({
     id:x.id,type:String(x.type||args.type),name:x.name,
     poster:typeof x.poster==='string'?x.poster:null,
     background:typeof x.background==='string'?x.background:null,
     logo:typeof x.logo==='string'?x.logo:null,
     description:typeof x.description==='string'?x.description:null
   }));
  return {key,addonId:args.addonId,addonName:args.addonName,catalogId:args.catalogId,
    catalogName:args.catalogName,type:args.type,items,
    hasMore:item.supportsSkip&&body.metas.length>0,nextSkip:skip+body.metas.length};
 }
 return {reload,page};
}
module.exports={createCatalogClient,catalogUrl,manifestUrl,safeAddonUrl};
