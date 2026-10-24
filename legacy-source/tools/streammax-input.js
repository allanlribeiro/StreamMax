/* Campo real: edição nativa, IME, seleção e área de transferência.
 * Protocolo polled por entrada_texto.c; nenhum callback entra no WASM. */
(function () {
  'use strict';
  var t = { seq: 0, valor: '', fim: -1, aberto: false, microfone: -1 };
  var tipo = 0, limite = 2048, painel, campo;
  function montar() {
    if (campo) return;
    var estilo = document.createElement('style');
    estilo.textContent = '#sm-editor[hidden]{display:none}#sm-editor{position:fixed;left:8%;right:8%;bottom:7%;z-index:10000;padding:24px;border:1px solid #41617a;border-radius:16px;background:#101a29;color:#f5f8ff;font:20px sans-serif;box-shadow:0 12px 48px #0009}#sm-editor label{display:block;margin-bottom:12px}#nvTextoCampo{box-sizing:border-box;width:100%;font:24px sans-serif;padding:14px;background:#080f1c;border:2px solid #51d5ff;border-radius:8px;color:white;caret-color:white;user-select:text;-webkit-user-select:text}#sm-editor .acoes{display:flex;justify-content:flex-end;margin-top:16px}#sm-editor button{font:20px sans-serif;padding:10px 22px;margin-left:12px;border-radius:8px;border:2px solid transparent;background:#213249;color:white}#sm-editor button:focus{border-color:#51d5ff;outline:none}#sm-editor button:last-child{background:#12678c}';
    document.head.appendChild(estilo);
    painel = document.createElement('section');
    painel.id = 'sm-editor'; painel.hidden = true;
    painel.setAttribute('role', 'dialog'); painel.setAttribute('aria-modal', 'true');
    painel.setAttribute('aria-labelledby', 'sm-editor-label');
    painel.innerHTML = '<label id="sm-editor-label" for="nvTextoCampo">Digite o texto</label><input id="nvTextoCampo" autocomplete="off" spellcheck="false"><div class="acoes"><button type="button" id="sm-editor-cancelar">Cancelar</button><button type="button" id="sm-editor-confirmar">Confirmar</button></div>';
    document.body.appendChild(painel); campo = document.getElementById('nvTextoCampo');
    ['input', 'change', 'compositionend'].forEach(function (ev) { campo.addEventListener(ev, ler); });
    document.getElementById('sm-editor-confirmar').onclick = function () { terminar(1); };
    document.getElementById('sm-editor-cancelar').onclick = function () { terminar(0); };
    ['keydown', 'keyup', 'keypress'].forEach(function (nome) {
      painel.addEventListener(nome, function (ev) {
        if (!t.aberto) return;
        ev.stopPropagation();
        if (nome !== 'keydown' || ev.isComposing || ev.keyCode === 229) return;
        if (ev.key === 'Escape' || ev.keyCode === 65385) { ev.preventDefault(); terminar(0); }
        else if (ev.keyCode === 65376 || (ev.key === 'Enter' && ev.target === campo)) { ev.preventDefault(); terminar(1); }
        else if (ev.key === 'Tab') {
          var botoes = painel.querySelectorAll('button');
          if (ev.shiftKey && ev.target === campo) { ev.preventDefault(); botoes[1].focus(); }
          else if (!ev.shiftKey && ev.target === botoes[1]) { ev.preventDefault(); campo.focus(); }
        }
      });
    });
  }
  function ler() {
    if (t.aberto && campo && campo.value !== t.valor) { t.valor = campo.value; t.seq++; }
  }
  function esconder() {
    if (painel) painel.hidden = true;
    var canvas = document.getElementById('canvas'); if (canvas) canvas.focus();
  }
  function terminar(ok) {
    if (!t.aberto) return;
    ler(); t.aberto = false; t.fim = ok; esconder();
  }
  t.configurar = function (novoTipo, max) { tipo = novoTipo; limite = Math.max(1, Math.min(max || 2048, 8192)); };
  t.abrir = function (valor) {
    montar(); t.valor = valor || ''; t.fim = -1; t.aberto = true; t.seq++;
    campo.type = tipo === 2 ? 'password' : 'text';
    campo.inputMode = tipo === 1 ? 'email' : 'text'; campo.maxLength = limite;
    document.getElementById('sm-editor-label').textContent = tipo === 2 ? 'Digite a senha' : tipo === 1 ? 'Digite o e-mail' : 'Digite o texto';
    campo.value = t.valor; painel.hidden = false; campo.focus();
    campo.setSelectionRange(campo.value.length, campo.value.length);
  };
  t.fechar = function () { t.aberto = false; esconder(); };
  window.addEventListener('keydown', function (ev) {
    if (t.aberto && ev.keyCode === 10009) { ev.preventDefault(); ev.stopImmediatePropagation(); terminar(0); }
  }, true);
  try {
    if (window.tizen && tizen.systeminfo) t.microfone = tizen.systeminfo.getCapability('http://tizen.org/feature/microphone') ? 1 : 0;
  } catch (_) {}
  window.nvTexto = t;
})();
