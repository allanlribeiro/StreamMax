(function () {
 'use strict';
 var bridge=window.streammaxDesktop;
 if(!bridge)return;
 var s={},external=false,headers='',tracks=[];
 bridge.onState(function(value){s=value;tracks=value.tracks||[];});
 bridge.onClose(function(){
   if(!external)return;
   var canvas=document.getElementById('canvas');if(canvas)canvas.focus();
   // Fechar a janela do player equivale a Voltar; preserva posição e duração.
   window.dispatchEvent(new KeyboardEvent('keydown',{key:'Escape',code:'Escape',keyCode:27,bubbles:true}));
   window.dispatchEvent(new KeyboardEvent('keyup',{key:'Escape',code:'Escape',keyCode:27,bubbles:true}));
 });
 window.smPlayer={
  active:function(){return external;},
  headers:function(value){headers=value;},
  open:function(url){external=true;s={open:true};bridge.open(url,headers);headers='';},
  stop:function(){if(external)bridge.close();external=false;},
  command:function(op,text,a){
   if(op==='pausar')bridge.command('pause',!!a);
   else if(op==='buscar')bridge.command('seek',a);
   else if(op==='volume')bridge.command('volume',a);
   else if(op==='faixa'){var list=tracks.filter(function(t){return t.type===(text==='AUDIO'?'audio':'sub');});if(list[a|0])bridge.command(text==='AUDIO'?'audio':'subtitle',list[a|0].id);}
   else if(op==='leg_mudo'&&a)bridge.command('subtitle','no');
   else if(op==='subtitle-url')bridge.command('subtitle-url',text);
   else if(op==='speed')bridge.command('speed',a);
  },
  state:function(){return [+s.pos||0,+s.duration||0,s.playing?1:0,s.ready?1:0,s.open?1:0,+s.width||0,+s.height||0,s.error?1:0,s.ended?1:0,0,0,s.error?1:0,tracks.length];},
  error:function(){return s.message||'';},
  tracks:function(){var audio=0,sub=0,out='';tracks.forEach(function(t){if(t.type!=='audio'&&t.type!=='sub')return;var i=t.type==='audio'?audio++:sub++;out+=(t.type==='audio'?'A':'T')+'\t'+i+'\t'+(t.lang||'')+'\t'+String(t.title||t.codec||'').replace(/[\t\n]/g,' ')+'\t-1\n';});return out;}
 };
})();
