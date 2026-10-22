'use strict';
const api=window.streammaxDesktop;
const $=id=>document.getElementById(id);
const auth=$('auth-panel'),home=$('home-panel'),account=$('account-panel');
let activeSourceRequest=null;
let requestSerial=0;
function closeDetails(){
 activeSourceRequest=null;
 $('detail-panel').hidden=true;
}
function openSources(item){
 activeSourceRequest='sources-'+Date.now()+'-'+(++requestSerial);
 const requestId=activeSourceRequest;
 $('sources-list').replaceChildren();
 $('sources-status').textContent='Buscando fontes nos seus addons…';
 $('sources-progress').value=0;
 $('sources-progress').max=1;
 api.searchSources(item.type,item.id,requestId).catch(error=>{
  if(activeSourceRequest===requestId)
    $('sources-status').textContent='Não foi possível buscar as fontes: '+String(error.message||error);
 });
}
api.onSourcesProgress(update=>{
 if(!activeSourceRequest||update.requestId!==activeSourceRequest||$('detail-panel').hidden)return;
 $('sources-progress').max=Math.max(1,update.total||1);
 $('sources-progress').value=Math.min(update.completed||0,Math.max(1,update.total||1));
 const count=Number(update.available)||0;
 const total=Number(update.total)||0;
 const completed=Number(update.completed)||0;
 $('sources-status').textContent= total===0
  ?'Nenhum addon de fontes foi encontrado na conta.'
  :update.phase==='complete'
   ?count+' fonte(s) encontrada(s) em '+total+' addon(s).'
   :'Fontes encontradas: '+count+' · Addons consultados: '+completed+'/'+total;
 if(Array.isArray(update.streams)){
  for(const source of update.streams){
   const row=document.createElement('li');
   const label=document.createElement('strong');
   label.textContent=source.title;
   const provider=document.createElement('span');
   provider.textContent=' · '+source.addonName+(source.playable?'':' · reprodução direta indisponível');
   row.append(label,provider);
   if(source.description){
    const description=document.createElement('p');
    description.textContent=source.description;
    row.append(description);
   }
   $('sources-list').append(row);
  }
 }
});
const view=window.StreamMaxHomeView.createHomeView({
 root:$('home-viewport'),
 onOpenDetails(item){
   $('detail-name').textContent=item.name;
   $('detail-description').textContent=item.description||'';
   $('detail-panel').hidden=false;
   $('detail-close').focus();
   openSources(item);
 }
});
let authorized=false;
let activePage='home';
let loginTimer=null;
let loginStartedAt=0;
let polling=false;
let catalogs=[];
let nextRowIndex=0;
let booting=false;
let renderGeneration=0;
function showPage(page){
 activePage=page==='account'?'account':'home';
 document.querySelectorAll('[data-page]').forEach(b=>{
  if(b.dataset.page===activePage)b.setAttribute('aria-current','page');
  else b.removeAttribute('aria-current');
 });
 account.hidden=activePage!=='account'||!authorized;
 home.hidden=activePage!=='home'||!authorized;
 auth.hidden=authorized||activePage==='account';
 if(!authorized){auth.hidden=false;account.hidden=true;home.hidden=true;}
}
function setAuth(title,description,message=''){
 $('auth-title').textContent=title;
 $('auth-description').textContent=description;
 $('auth-message').textContent=message;
}
function feedback(message){
 $('home-feedback').hidden=!message;
 $('home-feedback').textContent=message||'';
}
function stopPolling(){
 loginStartedAt=0;
 if(loginTimer!==null)clearTimeout(loginTimer);
 loginTimer=null;polling=false;
}
async function loadNextRows(){
 const currentGeneration=renderGeneration;
 const button=$('load-more-rows');
 button.disabled=true;button.hidden=true;
 const batch=catalogs.slice(nextRowIndex,nextRowIndex+8);
 nextRowIndex+=batch.length;
 if(!batch.length){button.disabled=false;return;}
 feedback(view.hasContent()?'':'Carregando as linhas da sua conta…');
 // Preserve Android row ordering even when remote providers respond out of order.
 const pages=await Promise.allSettled(batch.map(row=>api.catalogPage(row.key,0)));
 if(currentGeneration!==renderGeneration)return;
 for(let i=0;i<pages.length;i++){
  if(pages[i].status==='fulfilled')view.addRow({...pages[i].value,key:batch[i].key,title:batch[i].catalogName});
 }
 feedback(view.hasContent()?'':'Nenhum conteúdo disponível nestas linhas. Verifique seus addons ou tente novamente.');
 button.disabled=false;button.hidden=nextRowIndex>=catalogs.length;
}
async function loadHome(){
 const currentGeneration=++renderGeneration;
 showPage('home');
 $('app-status').textContent='Carregando catálogos…';
 view.clear();
 feedback('Sincronizando os catálogos da conta…');
 try{
  const response=await api.reloadCatalogs();
  if(currentGeneration!==renderGeneration)return;
  catalogs=Array.isArray(response.catalogs)?response.catalogs:[];
  nextRowIndex=0;
  if(!catalogs.length){
   feedback('Nenhum catálogo foi encontrado para o perfil principal. Verifique os addons configurados no StreamMax Android.');
  }else await loadNextRows();
  $('app-status').textContent='Conta conectada';
 }catch(error){
  feedback('Não foi possível carregar os catálogos da conta. '+String(error.message||error));
  $('app-status').textContent='Catálogos indisponíveis';
 }
}
async function onAuthenticated(){
 stopPolling();
 authorized=true;
 $('account-info').textContent='Sessão validada. Catálogos do perfil principal.';
 $('app-status').textContent='Conta conectada';
 await loadHome();
}
async function checkLogin(){
 if(polling)return;
 polling=true;
 try {
  const result=await api.authPoll();
  if(result.authenticated){await onAuthenticated();return;}
  if(['expired','denied','rejected'].includes(result.status)){
   stopPolling();
   setAuth('Autorização encerrada','Inicie uma nova autorização para acessar sua conta.');
   $('login-start').hidden=false;
   $('login-retry').hidden=true;
   return;
  }
  $('auth-message').textContent=result.status==='awaiting_secure_session'?
    'Autorização recebida, mas a sessão ainda não foi concluída.':'Aguardando autorização no navegador…';
 }catch(error){
  $('auth-message').textContent='Não foi possível verificar a autorização: '+String(error.message||error);
 }finally{
  polling=false;
  if(!authorized&&loginStartedAt&&Date.now()-loginStartedAt<5*60*1000){
   loginTimer=setTimeout(checkLogin,5000);
  }
 }
}
$('login-start').addEventListener('click',async()=>{
 stopPolling();
 $('login-start').disabled=true;
 setAuth('Preparando acesso','Gerando a autorização do StreamMax…');
 try{
  const challenge=await api.authStart();
  $('login-code-value').textContent=challenge.userCode;
  $('login-code').hidden=false;
  $('login-link').hidden=false;
  $('login-link').textContent=challenge.verificationUrl;
  $('login-start').hidden=true;
  $('login-open').hidden=false;
  $('login-retry').hidden=false;
  setAuth('Autorize este computador','Abra a página de autorização e confirme o código exibido abaixo.');
  loginStartedAt=Date.now();
  loginTimer=setTimeout(checkLogin,Math.max(3,Math.min(30,challenge.pollIntervalSeconds||5))*1000);
 }catch(error){
  setAuth('Não foi possível iniciar o login','Verifique a conexão e a configuração do servidor.',String(error.message||error));
  $('login-start').hidden=false;
 }finally{$('login-start').disabled=false;}
});
$('login-retry').addEventListener('click',()=>{stopPolling();loginStartedAt=Date.now();checkLogin();});
$('login-open').addEventListener('click',async()=>{
 try{await api.openLoginPage();}catch(error){$('auth-message').textContent=String(error.message||error);}
});
$('load-more-rows').addEventListener('click',loadNextRows);
document.querySelectorAll('[data-page]').forEach(button=>button.addEventListener('click',()=>showPage(button.dataset.page)));
$('signout').addEventListener('click',async()=>{
 stopPolling();
 await api.authSignOut();
 authorized=false;
 catalogs=[];
 nextRowIndex=0;
 view.clear();
 $('login-code').hidden=true;$('login-link').hidden=true;
 $('login-open').hidden=true;$('login-retry').hidden=true;
 $('login-start').hidden=false;
 setAuth('Entrar no StreamMax','Autorize este computador para sincronizar sua conta.');
 $('app-status').textContent='Sem sessão';
 showPage('home');
});
$('detail-close').addEventListener('click',closeDetails);
$('detail-panel').addEventListener('click',e=>{if(e.target===$('detail-panel'))closeDetails();});
document.addEventListener('keydown',event=>{
 if(event.key==='Escape'&&!$('detail-panel').hidden){closeDetails();event.preventDefault();}
});
async function boot(){
 if(booting)return;
 booting=true;
 try{
  const settings=await api.authStatus();
  if(!settings.configured){
   setAuth('Conexão não configurada','A edição Windows precisa da configuração de autenticação do mesmo servidor usado pelo Android. Nenhuma conta ou catálogo de demonstração será apresentado.');
   $('app-status').textContent='Servidor não configurado';
   return;
  }
  const result=await api.authRestore();
  if(result.authenticated){await onAuthenticated();return;}
  setAuth('Entrar no StreamMax','Use sua conta para sincronizar addons e catálogos do StreamMax Android.');
  $('app-status').textContent='Aguardando login';
  $('login-start').hidden=false;
  if(result.reason==='validation_unavailable')
   $('auth-message').textContent='A sessão anterior existe, mas não pôde ser validada. Verifique a conexão.';
 }catch(error){
  setAuth('Falha na inicialização','O aplicativo não conseguiu validar a autenticação.',String(error.message||error));
  $('app-status').textContent='Erro de conexão';
 }
}
boot();
