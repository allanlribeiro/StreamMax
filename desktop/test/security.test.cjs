const {test}=require('node:test');
const assert=require('node:assert/strict');
const {urlHttp,headers}=require('../mpv.cjs');
test('fontes só aceitam HTTP(S), sem execução de comandos',()=>{
 assert.equal(urlHttp('https://example.org/video.mkv?token=teste'),'https://example.org/video.mkv?token=teste');
 for(const url of ['file:///C:/Windows/a.exe','javascript:alert(1)','--script=ataque','data:text/plain,a'])assert.throws(()=>urlHttp(url));
});
test('cabeçalhos não permitem injeção de linhas ou valores inválidos',()=>{
 assert.deepEqual(headers('Authorization: Bearer teste\nReferer: https://example.org'),['Authorization: Bearer teste','Referer: https://example.org']);
 assert.throws(()=>headers('Referer: a\rX-Extra: b'));
 assert.throws(()=>headers('Opção sem dois pontos'));
});
