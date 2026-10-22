'use strict';
const api=window.streammaxDesktop;
const $=id=>document.getElementById(id);
const auth=$('auth-panel'),home=$('home-panel'),account=$('account-panel');
let activeSourceRequest=null;
let requestSerial=0;
const PLAYER_LABELS={
 idle:'Player parado',starting:'Abrindo player…',loading:'Carregando vídeo…',
 buffering:'Preparando buffer…',playing:'Reproduzindo',paused:'Pausado',
 ended:'Reprodução encerrada',error:'Falha no player',
 disconnected:'Player desconectado'
};
async function refreshPlayerStatus(){
 try{
  const status=await api.playerAvailability();
  $('mpv-status').textContent=status.available
   ?(PLAYER_LABELS[status.state]||'mpv disponível')
   :'Selecione o mpv.exe para reproduzir.';
  $('player-controls').hidden=!status.available||status.state==='idle';
 }catch(error){
  $('mpv-status').textContent='Não foi possível verificar o mpv: '+String(error.message||error);
 }
}
api.onPlayerStatus(event=>{
 if(!event||typeof event.state!=='string')return;
 $('mpv-status').textContent=PLAYER_LABELS[event.state]||'Player iniciado';
 $('player-controls').hidden=['idle','error','disconnected'].includes(event.state);
});
$('mpv-select').addEventListener('click',async()=>{
 try{
  const selected=await api.selectMpv();
  $('mpv-status').textContent=selected.available
   ?'mpv configurado. Selecione uma fonte direta para reproduzir.'
   :'O mpv ainda não foi selecionado.';
 }catch(error){$('mpv-status').textContent='Não foi possível configurar: '+String(error.message||error);}
});
$('player-stop').addEventListener('click',async()=>{
 try{await api.stopPlayer();}catch(error){$('mpv-status').textContent=String(error.message||error);}
});
document.querySelectorAll('[data-player-command]').forEach(button=>{
 button.addEventListener('click',async()=>{
  try{await api.playerCommand(button.dataset.playerCommand);}
  catch(error){$('mpv-status').textContent=String(error.message||error);}
 });
});
refreshPlayerStatus();
let detailGeneration=0;
let episodeSeasons=[];
let visibleEpisodes=[];
let detailsItem=null;
function closeDetails(){
 detailGeneration++;
 activeSourceRequest=null;
 detailsItem=null;
 episodeSeasons=[];
 visibleEpisodes=[];
 $('detail-panel').hidden=true;
}
function isSeriesType(type){return type==='series';}
function renderEpisodeChoice(){
 const season=episodeSeasons[Number($('season-select').value)];
 visibleEpisodes=(detailsItem?.videos||[])
  .filter(ep=>(ep.season??null)===(season??null))
  .sort((a,b)=>(a.episode??0)-(b.episode??0));
 const select=$('episode-select');
 select.replaceChildren();
 visibleEpisodes.forEach((ep,index)=>{
   const opt=document.createElement('option');
   opt.value=String(index);
   opt.textContent='E'+String(ep.episode??(index+1)).padStart(2,'0')+(ep.title?' — '+ep.title:'');
   select.append(opt);
 });
 renderEpisodeSummary();
}
function renderEpisodeSummary(){
 const ep=visibleEpisodes[Number($('episode-select').value)];
 $('episode-summary').textContent=ep?.overview||ep?.title||'';
 $('episode-search').disabled=!ep;
}
async function showDetails(item){
 const myGeneration=++detailGeneration;
 activeSourceRequest=null;
 episodeSeasons=[];
 visibleEpisodes=[];
 detailsItem=null;
 $('detail-name').textContent=item.name;
 $('detail-description').textContent=item.description||'';
 $('detail-facts').textContent=item.releaseInfo||'';
 $('detail-meta-status').textContent='Carregando informações do título…';
 $('detail-episodes').hidden=true;
 $('sources-list').replaceChildren();
 $('sources-status').textContent='Preparando pesquisa de fontes…';
 $('sources-progress').value=0;
 $('sources-progress').max=1;
 $('detail-panel').hidden=false;
 $('detail-close').focus();
 try{
   if(!item.catalogKey)throw new Error('Catálogo de origem indisponível');
   const loaded=await api.loadMetadata(item.catalogKey,item.type,item.id);
   if(myGeneration!==detailGeneration||$('detail-panel').hidden)return;
   const meta={...item,...loaded};
   detailsItem=meta;
   $('detail-name').textContent=meta.name||item.name;
   $('detail-description').textContent=meta.description||item.description||'';
   $('detail-facts').textContent=[meta.releaseInfo,meta.runtime,
     Array.isArray(meta.genres)?meta.genres.join(', '):''].filter(Boolean).join(' · ');
   $('detail-meta-status').textContent='';
   if(isSeriesType(meta.type)){
     if(!Array.isArray(meta.videos)||!meta.videos.length){
       $('sources-status').textContent='Esta série não possui episódios publicados no addon de origem.';
       return;
     }
     episodeSeasons=Array.from(new Set(meta.videos.map(ep=>ep.season??null)))
       .sort((a,b)=>(a??-1)-(b??-1));
     const seasonSelect=$('season-select');
     seasonSelect.replaceChildren();
     episodeSeasons.forEach((season,index)=>{
       const opt=document.createElement('option');
       opt.value=String(index);
       opt.textContent=season===null?'Outros episódios':'Temporada '+season;
       seasonSelect.append(opt);
     });
     $('detail-episodes').hidden=false;
     renderEpisodeChoice();
     $('sources-status').textContent='Selecione o episódio para encontrar fontes.';
   }else if(meta.type==='movie'){
     openSources({type:meta.type,id:meta.id});
   }else{
     $('sources-status').textContent='Esse tipo de conteúdo ainda não está disponível na edição Windows.';
   }
 }catch(error){
   if(myGeneration!==detailGeneration||$('detail-panel').hidden)return;
   $('detail-meta-status').textContent='Informações completas indisponíveis; exibindo dados do catálogo.';
   if(isSeriesType(item.type)){
     $('sources-status').textContent='Não foi possível recuperar os episódios desta série.';
   }else if(item.type==='movie')openSources(item);
   else $('sources-status').textContent='Esse tipo de conteúdo ainda não está disponível na edição Windows.';
 }
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
   if(source.playable){
    const play=document.createElement('button');
    play.type='button';
    play.className='secondary source-play';
    play.textContent='Reproduzir no mpv';
    play.addEventListener('click',async()=>{
     play.disabled=true;
     try{
      const capability=await api.playerAvailability();
      if(!capability.available){
       $('mpv-status').textContent='Selecione primeiro o mpv.exe no botão Configurar.';
       $('mpv-select').focus();
       return;
      }
      $('mpv-status').textContent='Iniciando o mpv…';
      await api.playSource(source.id);
     }catch(error){
      $('mpv-status').textContent='Não foi possível iniciar a fonte: '+String(error.message||error);
     }finally{play.disabled=false;}
    });
    row.append(play);
   }
   $('sources-list').append(row);
  }
 }
});
const view=window.StreamMaxHomeView.createHomeView({
 root:$('home-viewport'),
 onOpenDetails: showDetails
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
$('season-select').addEventListener('change',renderEpisodeChoice);
$('episode-select').addEventListener('change',renderEpisodeSummary);
$('episode-search').addEventListener('click',()=>{
 const episode=visibleEpisodes[Number($('episode-select').value)];
 if(!episode)return;
 openSources({type:'series',id:episode.id});
});
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
