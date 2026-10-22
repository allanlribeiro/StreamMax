const pages = {
  home: ['Bem-vindo ao StreamMax','Versão experimental para Windows. A integração dos catálogos, fontes e contas do aplicativo Android ainda está em desenvolvimento.'],
  continue: ['Continuar assistindo','A sincronização do histórico será implementada ao conectar a camada de dados do StreamMax.'],
  settings: ['Configurações','As preferências de áudio, legendas, player e janela serão adicionadas nesta edição.']
};
const player=document.getElementById('player');
const status=document.getElementById('playback-status');
function go(view) {
  const [title, description] = pages[view] || pages.home;
  document.getElementById('title').textContent = title;
  document.getElementById('description').textContent = description;
  document.querySelectorAll('[data-view]').forEach(b => b.classList.toggle('active',b.dataset.view===view));
}
document.querySelectorAll('[data-view]').forEach(b=>b.addEventListener('click',()=>go(b.dataset.view)));
document.getElementById('open-video').addEventListener('click',async()=>{
  const url=await window.streammaxDesktop.openLocalVideo();
  if(!url)return;
  player.src=url;document.getElementById('player-section').hidden=false;
  status.textContent='Abrindo vídeo local…';
  try { await player.play();status.textContent='';}
  catch {status.textContent='Pressione reproduzir para iniciar.';}
});
player.addEventListener('error',()=>{status.textContent='Formato ou codec não suportado pelo reprodutor experimental. A compatibilidade será ampliada.';});
document.addEventListener('keydown',e=>{
  if(['INPUT','TEXTAREA','SELECT'].includes(document.activeElement.tagName))return;
  if(e.key==='Escape')go('home');
  if(document.getElementById('player-section').hidden)return;
  if(e.key===' ' && document.activeElement.tagName!=='BUTTON'){e.preventDefault();player.paused?player.play():player.pause();}
  if(e.key==='ArrowLeft' && document.activeElement.tagName!=='BUTTON'){e.preventDefault();player.currentTime=Math.max(0,player.currentTime-5);}
  if(e.key==='ArrowRight' && document.activeElement.tagName!=='BUTTON'){e.preventDefault();player.currentTime=Math.min(player.duration||Infinity,player.currentTime+5);}
});
