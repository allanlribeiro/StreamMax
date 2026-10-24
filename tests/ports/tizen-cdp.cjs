const {chromium}=require('playwright');
const fs=require('node:fs');
const {inputTests}=require('./input.cjs');
const {loginInput}=require('./login-input.cjs');
(async()=>{
 let browser;
 for(let i=0;i<45;i++){try{browser=await chromium.connectOverCDP('http://127.0.0.1:9222');break;}catch(_){await new Promise(r=>setTimeout(r,1000));}}
 if(!browser)throw Error('Emulador não expôs o depurador');
 const pages=browser.contexts().flatMap(c=>c.pages());
 const page=pages.find(p=>p.url().includes('index.html'))||pages[0];
 if(!page)throw Error('Aplicativo não abriu no emulador');
 const log=[];page.on('console',m=>log.push(m.text()));
 try{
  await page.waitForFunction(()=>window.__nvPrimeiroQuadro===1,null,{timeout:180000});
  await page.screenshot({path:'output/tizen-emulador/aplicativo.png'});
  await loginInput(page,'output/tizen-emulador');
  await inputTests(page,'output/tizen-emulador');
  fs.writeFileSync('output/tizen-emulador/ambiente.json',JSON.stringify(await page.evaluate(()=>({userAgent:navigator.userAgent,avplay:!!(window.webapis&&webapis.avplay),sharedArrayBuffer:typeof SharedArrayBuffer!=='undefined'})),null,2));
 }finally{fs.writeFileSync('output/tizen-emulador/console.txt',log.join('\n'));await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
