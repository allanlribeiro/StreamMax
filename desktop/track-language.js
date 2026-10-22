'use strict';
// Portuguese regional classification is conservative: "por" is NOT pt-BR.
function languageLabel(language,title=''){
 const lang=String(language||'').trim().toLowerCase().replace(/_/g,'-');
 const t=String(title||'').toLowerCase();
 const pt=/^(pt|por|portuguese)(-|$)/.test(lang)||/portugu[êe]s|portuguese/i.test(t);
 if(!pt)return lang?lang.toUpperCase():'Idioma não informado';
 if(/^(pt|por|portuguese)-(br|bra)$/.test(lang)||/brasil|brazil/i.test(t))
  return 'Português (Brasil)';
 if(/^(pt|por|portuguese)-(pt|prt)$/.test(lang)||/portugal|european portuguese|portugu[êe]s europeu/i.test(t))
  return 'Português (Portugal)';
 return 'Português (variante não informada)';
}
function safeTracks(value){
 if(!Array.isArray(value))return [];
 return value.slice(0,128).filter(v=>v&&typeof v==='object'&&
  ['audio','sub'].includes(v.type)&&Number.isInteger(v.id)&&v.id>=0&&v.id<=1000000)
  .map(v=>({
   id:v.id,type:v.type,
   language:languageLabel(v.lang,v.title),
   title:typeof v.title==='string'?v.title.slice(0,120):'',
   selected:v.selected===true,
   forced:v.forced===true
  }));
}
module.exports={languageLabel,safeTracks};
