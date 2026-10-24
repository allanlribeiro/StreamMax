// Bancada Chromium: NÃO representa o emulador Tizen.
const {chromium}=require('playwright');
const fs=require('node:fs');
const path=require('node:path');
const {inputTests}=require('./input.cjs');
(async()=>{
 const out=path.resolve('output/navegador');fs.mkdirSync(out,{recursive:true});
 const browser=await chromium.launch({executablePath:process.env.CHROMIUM_PATH||'/usr/bin/chromium',headless:true,args:['--no-sandbox','--enable-unsafe-swiftshader']});
 try{
  const page=await browser.newPage({viewport:{width:1440,height:810}});
  if(process.env.STREAMMAX_URL){await page.goto(process.env.STREAMMAX_URL);await page.waitForFunction(()=>window.__nvPrimeiroQuadro===1,null,{timeout:180000});await page.waitForTimeout(2500);await page.screenshot({path:out+'/aplicativo.png'});}
  else {await page.setContent('<html lang="pt-BR"><head></head><body style="background:#0e0f12"><canvas id="canvas" tabindex="0"></canvas></body></html>');await page.addScriptTag({path:'legacy-source/tools/streammax-input.js'});}
  await inputTests(page,out);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
