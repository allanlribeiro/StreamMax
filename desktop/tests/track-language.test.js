const test=require('node:test');
const assert=require('node:assert/strict');
const {languageLabel,safeTracks}=require('../track-language');

test('keeps pt-BR and pt-PT separate when the region is provided',()=>{
 assert.equal(languageLabel('pt-BR'),'Português (Brasil)');
 assert.equal(languageLabel('pt-PT'),'Português (Portugal)');
 assert.equal(languageLabel('por-BR'),'Português (Brasil)');
 assert.equal(languageLabel('por-PT'),'Português (Portugal)');
 assert.equal(languageLabel('por','Português Brasil'),'Português (Brasil)');
 assert.equal(languageLabel('por','Português Portugal'),'Português (Portugal)');
});
test('never guesses Brazilian Portuguese from generic metadata',()=>{
 assert.equal(languageLabel('pt'),'Português (variante não informada)');
 assert.equal(languageLabel('por'),'Português (variante não informada)');
 assert.equal(languageLabel('', 'Portuguese'),'Português (variante não informada)');
 assert.equal(languageLabel('eng'),'ENG');
 assert.equal(languageLabel('', ''),'Idioma não informado');
});
test('only exposes track details safe for UI, without local filenames',()=>{
 const tracks=safeTracks([
  {id:1,type:'audio',lang:'pt-BR',title:'Dublado',selected:true,'external-filename':'C:\\secrets\\audio.aac'},
  {id:3,type:'sub',lang:'por',title:'Legenda',selected:false,external:true,'external-filename':'https://private.example/sub.srt?secret=123'},
  {id:7,type:'video',title:'hidden video'},
  {id:'4',type:'sub',lang:'pt-PT'}
 ]);
 assert.equal(tracks.length,2);
 assert.equal(tracks[0].language,'Português (Brasil)');
 assert.equal(tracks[1].language,'Português (variante não informada)');
 assert.equal(tracks[0].selected,true);
 assert.ok(tracks.every(t=>!('external-filename' in t)&&!('external' in t)));
 assert.ok(!JSON.stringify(tracks).includes('secret=123'));
});
