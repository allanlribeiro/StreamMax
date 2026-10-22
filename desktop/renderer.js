const pages = {
  home: ['Bem-vindo ao StreamMax','Estrutura inicial para adaptar o StreamMax ao Windows com mouse, teclado e janelas. O catálogo, login e player ainda precisam ser integrados à base existente.'],
  continue: ['Continuar assistindo','A sincronização do histórico será implementada ao conectar a camada de dados do StreamMax.'],
  settings: ['Configurações','As preferências de áudio, legendas, player e janela serão adicionadas nesta edição.']
};
function go(view) {
  const [title, description] = pages[view] || pages.home;
  document.getElementById('title').textContent = title;
  document.getElementById('description').textContent = description;
  document.querySelectorAll('[data-view]').forEach(b => {b.classList.toggle('active',b.dataset.view===view);});
}
document.querySelectorAll('[data-view]').forEach(b => b.addEventListener('click',()=>go(b.dataset.view)));
document.addEventListener('keydown',e => { if(e.key==='Escape')go('home'); });
