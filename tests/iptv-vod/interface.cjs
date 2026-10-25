// Interface C real compilada para WASM + provedor de teste local.
// Nao equivale a medir fluidez/decodificacao na TCL ou na Samsung.
const {chromium}=require('playwright');
const fs=require('node:fs');
const path=require('node:path');
const assert=require('node:assert/strict');
const out=path.resolve('output/iptv-vod');fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(out+'/requests.jsonl','');
const requests=()=>fs.readFileSync(out+'/requests.jsonl','utf8').trim().split('\n').filter(Boolean).map(JSON.parse);
(async()=>{
 const browser=await chromium.launch({executablePath:'/usr/bin/chromium',headless:true,
  args:['--no-sandbox','--enable-unsafe-swiftshader','--autoplay-policy=no-user-gesture-required']});
 const context=await browser.newContext({viewport:{width:1920,height:1080}});
 // Servicos externos sao substituidos por indisponibilidade, sem usar contas.
 await context.route('**/*',async route=>{
  const u=new URL(route.request().url());
  if(u.origin==='http://127.0.0.1:8788')return route.continue();
  // Provedor HTTPS ficticio: evita o proxy HTTP herdado do backend VIDAA.
  if(u.origin==='https://fixture-iptv.example.invalid'){
   const response=await context.request.get('http://127.0.0.1:8788'+u.pathname+u.search,
    {headers:route.request().headers()});
   return route.fulfill({response,headers:{...response.headers(),
    'Access-Control-Allow-Origin':'*','Cross-Origin-Resource-Policy':'cross-origin'}});
  }
  return route.fulfill({status:503,contentType:'application/json',body:'{}',
    headers:{'Access-Control-Allow-Origin':'*','Cross-Origin-Resource-Policy':'cross-origin'}});
 });
 const page=await context.newPage();const errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 const logs=[];page.on('console',m=>{if(m.type()==='log') logs.push(m.text());});
 try{
  await page.goto('http://127.0.0.1:8788/blank');
  const jwt='eyJhbGciOiJub25lIn0.'+Buffer.from(JSON.stringify({sub:'fixture-vod-local',exp:4102444800})).toString('base64url')+'.fixture';
  const marks=new Set();
  for(const name of fs.readdirSync('legacy-source/src').filter(n=>/^novidades.*\.[ch]$/.test(n)))
   for(const m of fs.readFileSync('legacy-source/src/'+name,'utf8').matchAll(/"(novidades[^"\n]*\.txt)"/g))marks.add(m[1]);
  await page.evaluate(async({jwt,marks})=>{
   await new Promise((resolve,reject)=>{
    const req=indexedDB.open('/nuvio',21);
    req.onupgradeneeded=()=>{let store=req.result.createObjectStore('FILE_DATA');store.createIndex('timestamp','timestamp',{unique:false});};
    req.onerror=()=>reject(req.error);
    req.onsuccess=()=>{
     const db=req.result,tx=db.transaction('FILE_DATA','readwrite'),s=tx.objectStore('FILE_DATA');
     const enc=new TextEncoder(),date=new Date();
     s.put({timestamp:date,mode:16877},'/nuvio');
     const files={
      'sessao.txt':jwt+'\nrefresh-fixture\n0\n','perfil.txt':'1\n',
      'perfis.txt':'1\t0\t1\t0\t#397BFF\tTeste VOD\t\t\n',
      'xtream-p1.txt':'servidor\thttps://fixture-iptv.example.invalid\nusuario\tfixture\nsenha\tteste-local\n',
      'telemetria-perguntado.txt':'0\n','novidades-20-guia.txt':'1\n',
      'novidades-180-ui.txt':'1\n','novidades-207.txt':'1\n',
      'diagnostico-otimizacao-intro.cfg':'1\n','salvos-intro.txt':'1\n',
      'aviso-log.txt':'1\n','aviso-legal.txt':'1\n','pip-modo.txt':'0\n',
      'recintro-social.txt':'1\n',
      'dicas-controle-205.txt':'1\n',
     };
     for(const name of marks)files[name]='1\n';
     for(const [name,text]of Object.entries(files))s.put({timestamp:date,mode:33188,contents:enc.encode(text)},'/nuvio/'+name);
     tx.oncomplete=()=>{db.close();resolve();};tx.onerror=()=>reject(tx.error);
    };
   });
  },{jwt,marks:[...marks]});
  await page.goto('http://127.0.0.1:8788/');
  await page.waitForFunction(()=>window.__nvPrimeiroQuadro===1,null,{timeout:180000});
  await page.waitForTimeout(3500);
  await page.screenshot({path:out+'/00-inicial.png'});
  await page.locator('#canvas').focus();
  for(let tentativa=0;tentativa<8;tentativa++){
   if(tentativa)await page.keyboard.press('Escape');
   await page.keyboard.press('F10');await page.waitForTimeout(1200);
   if(requests().some(r=>r.action==='get_vod_categories'))break;
  }
  await page.keyboard.press('l');await page.waitForTimeout(2000);
  assert.ok(requests().some(r=>r.action==='get_vod_categories'),'O guia precisa consultar o Xtream');
  await page.screenshot({path:out+'/01-ao-vivo.png'});
  const click=async(x,y)=>{await page.mouse.move(x,y);await page.waitForTimeout(80);await page.mouse.click(x,y);await page.waitForTimeout(600);};
  await click(668,57);await page.keyboard.press('ArrowDown');await page.waitForTimeout(2500);
  assert.ok(requests().some(r=>r.action==='get_vod_streams'),'Filmes deve carregar o VOD');
  await page.screenshot({path:out+'/02-filmes.png'});
  for(let i=0;i<7;i++)await page.keyboard.press('ArrowDown');
  await page.waitForTimeout(600);await page.screenshot({path:out+'/03-rolagem.png'});
  for(let i=0;i<8;i++)await page.keyboard.press('ArrowUp');
  await page.keyboard.press('ArrowRight');await page.keyboard.press('ArrowDown');await page.waitForTimeout(2500);
  assert.ok(requests().some(r=>r.action==='get_series'),'Series deve consultar o provedor');
  await page.screenshot({path:out+'/04-series.png'});
  await page.keyboard.press('Enter');await page.waitForTimeout(2500);
  assert.ok(requests().some(r=>r.action==='get_series_info'),'OK deve abrir os episodios');
  await page.screenshot({path:out+'/05-episodios.png'});
  // T2 pela coluna de temporadas; nenhum video deve abrir por navegar.
  await page.keyboard.press('ArrowLeft');await page.keyboard.press('ArrowDown');
  await page.keyboard.press('ArrowRight');await page.waitForTimeout(600);
  await page.screenshot({path:out+'/06-temporada-2.png'});
  await page.keyboard.press('Enter');await page.waitForTimeout(3500);
  await page.screenshot({path:out+'/07-reproducao.png'});
  const video=await page.evaluate(()=>Array.from(document.querySelectorAll('video')).map(v=>({src:v.currentSrc,time:v.currentTime,paused:v.paused})));
  fs.writeFileSync(out+'/resultado-parcial.json',JSON.stringify({errors,video},null,2));
  fs.writeFileSync(out+'/console.txt',logs.join('\n'));
  assert.equal(errors.length,0);
  assert.ok(video.some(v=>v.src.includes('/series/')&&v.src.includes('/902.mp4')&&v.time>0),'O episodio selecionado precisa reproduzir');
 }finally{fs.writeFileSync(out+'/console.txt',logs.join('\n'));await page.screenshot({path:out+'/estado-final.png'}).catch(()=>{});await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
