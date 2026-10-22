'use strict';
const api=window.streammaxDesktop;
const $=id=>document.getElementById(id);
const auth=$('auth-panel'),home=$('home-panel'),account=$('account-panel');
let activeSourceRequest=null;
let activePlaybackMetadata=null;
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
function updateTrackSelect(elementId,tracks,kind){
 const selector=$(elementId);
 selector.replaceChildren();
 if(kind==='sub'){
  const none=document.createElement('option');
  none.value='no';none.textContent='Desativada';
  selector.append(none);
 }
 const list=tracks.filter(track=>track.type===kind);
 for(const track of list){
  const option=document.createElement('option');
  option.value=String(track.id);
  option.textContent=track.language+(track.title?' · '+track.title:'')+(track.forced?' · Forçada':'');
  option.selected=track.selected;
  selector.append(option);
 }
 if(!list.length&&kind==='audio'){
  const option=document.createElement('option');
  option.value='';option.textContent='Nenhuma faixa disponível';
  selector.append(option);
 }
 selector.disabled=!list.length&&kind==='audio';
}
api.onPlayerTracks(event=>{
 const tracks=Array.isArray(event?.tracks)?event.tracks:[];
 updateTrackSelect('player-audio-track',tracks,'audio');
 updateTrackSelect('player-subtitle-track',tracks,'sub');
});
for(const [id,kind] of [['player-audio-track','audio'],['player-subtitle-track','sub']]){
 $(id).addEventListener('change',async()=>{
  const value=$(id).value;
  if(!value)return;
  try{await api.playerTrack(kind,value==='no'?'no':Number(value));}
  catch(error){$('mpv-status').textContent='Falha na seleção de faixa: '+String(error.message||error);}
 });
}
function formatTime(ms){
 if(!Number.isFinite(ms)||ms<0)return '0:00';
 const seconds=Math.floor(ms/1000);
 return Math.floor(seconds/60)+':'+String(seconds%60).padStart(2,'0');
}
api.onPlayerProgress(event=>{
 if(!event||typeof event!=='object')return;
 const position=Number(event.positionMs)||0;
 const duration=Number(event.durationMs)||0;
 $('playback-timeline').textContent=duration>0
  ?formatTime(position)+' / '+formatTime(duration)+
    (Number.isFinite(event.bufferedSeconds)?' · Buffer '+Math.round(event.bufferedSeconds)+'s':'')
  :'';
});
api.onPlayerWarning(event=>{
 if(event?.message)$('mpv-status').textContent=String(event.message);
});
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
 activePlaybackMetadata=null;
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
     openSources({type:meta.type,id:meta.id},{
      contentId:meta.id,contentType:'movie',name:meta.name,
      poster:meta.poster||'',backdrop:meta.background||''
     });
   }else{
     $('sources-status').textContent='Esse tipo de conteúdo ainda não está disponível na edição Windows.';
   }
 }catch(error){
   if(myGeneration!==detailGeneration||$('detail-panel').hidden)return;
   $('detail-meta-status').textContent='Informações completas indisponíveis; exibindo dados do catálogo.';
   if(isSeriesType(item.type)){
     $('sources-status').textContent='Não foi possível recuperar os episódios desta série.';
   }else if(item.type==='movie')openSources(item,{
     contentId:item.id,contentType:'movie',name:item.name,
     poster:item.poster||'',backdrop:item.background||''
    });
   else $('sources-status').textContent='Esse tipo de conteúdo ainda não está disponível na edição Windows.';
 }
}
function openSources(item,metadata=null){
 activePlaybackMetadata=metadata;
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
      const metadata=activePlaybackMetadata;
      if(!metadata)throw new Error('Informações do título indisponíveis');
      const result=await api.playSource(source.id,metadata);
      if(result.resumeAvailable)$('mpv-status').textContent=result.resumeSource==='android-account'?
        'Retomando posição salva na TV…':'Retomando posição salva neste computador…';
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
async function loadContinueWatching(){
 const list=$('continue-list');
 list.replaceChildren();
 $('continue-message').textContent='Carregando histórico local…';
 try{
  const records=await api.listWatchProgress();
  if(!authorized)return;
  if(!records.length){
   $('continue-message').textContent='Nenhum conteúdo iniciado neste computador ainda.';
   return;
  }
  $('continue-message').textContent=records.length+' conteúdo(s) para continuar.';
  for(const record of records){
   const card=document.createElement('button');
   card.type='button';card.className='continue-card';
   const art=document.createElement('img');
   art.alt='';art.loading='lazy';art.className='continue-card__image';
   if(typeof record.poster==='string'&&record.poster.startsWith('https://'))art.src=record.poster;
   else art.hidden=true;
   const meta=document.createElement('div');
   const title=document.createElement('strong');
   title.textContent=record.name||record.contentId;
   const episode=document.createElement('span');
   episode.textContent=record.contentType==='series'
    ?'T'+record.season+' E'+record.episode+(record.episodeTitle?' · '+record.episodeTitle:'')
    :'Filme';
   const pct=Math.min(100,Math.round(100*record.position/record.duration));
   const label=document.createElement('small');
   label.textContent=formatTime(record.position)+' de '+formatTime(record.duration)+' · '+pct+'%';
   const progress=document.createElement('progress');
   progress.max=100;progress.value=pct;
   meta.append(title,episode,label,progress);
   card.append(art,meta);
   card.addEventListener('click',()=>{
    activeSourceRequest=null;activePlaybackMetadata=null;
    $('detail-name').textContent=record.name||record.contentId;
    $('detail-description').textContent=record.episodeTitle||'';
    $('detail-facts').textContent='Retomar de '+formatTime(record.position);
    $('detail-meta-status').textContent='Histórico deste computador';
    $('detail-episodes').hidden=true;
    $('detail-panel').hidden=false;
    $('detail-close').focus();
    openSources({type:record.contentType,id:record.videoId},{
     contentId:record.contentId,contentType:record.contentType,
     name:record.name,episodeTitle:record.episodeTitle,
     season:record.season,episode:record.episode,
     poster:record.poster,backdrop:record.backdrop
    });
   });
   list.append(card);
  }
 }catch(error){
  $('continue-message').textContent='Não foi possível ler o histórico: '+String(error.message||error);
 }
}
$('continue-refresh').addEventListener('click',loadContinueWatching);
let remoteHistoryGeneration=0;
async function loadRemoteHistory(){
 const generation=++remoteHistoryGeneration;
 const list=$('remote-progress-list');
 list.replaceChildren();
 $('remote-progress-refresh').disabled=true;
 $('remote-progress-status').textContent='Consultando histórico sincronizado…';
 try{
  const records=await api.listRemoteProgress();
  if(generation!==remoteHistoryGeneration||!authorized)return;
  $('remote-progress-status').textContent=records.length
   ?records.length+' título(s) disponíveis na prévia da conta.'
   :'Nenhum conteúdo em andamento foi encontrado no histórico da TV.';
  for(const record of records){
   // Remote sync RPC returns IDs and watch positions, not names or artwork.
   // Preserve that distinction; do not invent titles or imply local import.
   const card=document.createElement('button');
   card.type='button';
   card.className='continue-card remote-progress-entry';
   card.title='Buscar fonte e retomar a posição salva na TV';
   const metadata=document.createElement('div');
   const title=document.createElement('strong');
   title.textContent=record.contentId;
   const details=document.createElement('span');
   details.textContent=record.contentType==='series'
    ?'Série · T'+record.season+' E'+record.episode
    :'Filme';
   const label=document.createElement('small');
   label.textContent=formatTime(record.position)+' de '+formatTime(record.duration);
   const progress=document.createElement('progress');
   progress.max=100;
   progress.value=Math.min(100,Math.round(100*record.position/record.duration));
   metadata.append(title,details,label,progress);
   card.append(metadata);
   card.addEventListener('click',()=>{
    ++detailGeneration;
    activeSourceRequest=null;
    activePlaybackMetadata=null;
    detailsItem=null;
    $('detail-name').textContent=record.contentId;
    $('detail-description').textContent=record.contentType==='series'?
      'Temporada '+record.season+' · Episódio '+record.episode:'Filme';
    $('detail-facts').textContent='Histórico da TV · Retomar de '+formatTime(record.position);
    $('detail-meta-status').textContent='Título e capa ainda não recuperados para esta prévia.';
    $('detail-episodes').hidden=true;
    $('detail-panel').hidden=false;
    $('detail-close').focus();
    openSources({type:record.contentType,id:record.videoId},{
      contentId:record.contentId,contentType:record.contentType,
      name:record.contentId,episodeTitle:'',
      season:record.season,episode:record.episode,
      poster:'',backdrop:''
    });
   });
   list.append(card);
  }
 }catch(error){
  if(generation===remoteHistoryGeneration&&authorized)
   $('remote-progress-status').textContent='Histórico da TV indisponível: '+String(error.message||error);
 }finally{
  if(generation===remoteHistoryGeneration)$('remote-progress-refresh').disabled=false;
 }
}
$('remote-progress-refresh').addEventListener('click',loadRemoteHistory);


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
 activePage=['account','continue'].includes(page)?page:'home';
 document.querySelectorAll('[data-page]').forEach(b=>{
  if(b.dataset.page===activePage)b.setAttribute('aria-current','page');
  else b.removeAttribute('aria-current');
 });
 account.hidden=activePage!=='account'||!authorized;
 home.hidden=activePage!=='home'||!authorized;
 $('continue-panel').hidden=activePage!=='continue'||!authorized;
 auth.hidden=authorized||activePage==='account';
 if(!authorized){auth.hidden=false;account.hidden=true;home.hidden=true;$('continue-panel').hidden=true;}
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
document.querySelectorAll('[data-page]').forEach(button=>button.addEventListener('click',()=>{
 showPage(button.dataset.page);
 if(button.dataset.page==='continue'&&authorized)void loadContinueWatching();
}));
$('signout').addEventListener('click',async()=>{
 ++remoteHistoryGeneration;
 $('remote-progress-list').replaceChildren();
 $('remote-progress-status').textContent='';
 stopPolling();
 await api.authSignOut();
 authorized=false;
 catalogs=[];
 nextRowIndex=0;
 view.clear();
 $('continue-list').replaceChildren();
 $('continue-message').textContent='';
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
$('server-save').addEventListener('click',async()=>{
 const button=$('server-save');
 button.disabled=true;
 $('server-config-message').textContent='Validando a conexão…';
 try{
  const result=await api.saveServerConfig({
   endpoint:$('server-endpoint').value,
   publicKey:$('server-pubkey').value,
   redirectUrl:$('server-redirect').value
  });
  $('server-pubkey').value='';
  $('server-config-message').textContent=result.restarting?
   'Conexão salva. O StreamMax será reiniciado.':
   'Configuração salva.';
 }catch(error){
  $('server-config-message').textContent=String(error.message||error);
 }finally{button.disabled=false;}
});
async function boot(){
 if(booting)return;
 booting=true;
 try{
  const settings=await api.authStatus();
  if(!settings.configured){
   if(settings.configSource==='invalid_environment'){
    setAuth('Configuração do Windows inválida',
      'As variáveis de conexão do Windows estão incorretas. Corrija STREAMMAX_SUPABASE_URL, STREAMMAX_SUPABASE_PUBLIC_KEY e STREAMMAX_LOGIN_REDIRECT_URL antes de abrir o aplicativo.');
    $('server-config-panel').hidden=true;
    $('app-status').textContent='Variáveis inválidas';
   }else{
    setAuth('Conexão não configurada','Para utilizar sua conta e seus addons, configure o servidor público do mesmo StreamMax que você usa no Android.');
    $('server-config-panel').hidden=false;
    $('app-status').textContent=settings.configSource==='invalid'?
     'Configuração do servidor inválida':'Servidor não configurado';
   }
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
