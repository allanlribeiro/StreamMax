const {_electron}=require('playwright');
const path=require('node:path');
const fs=require('node:fs');
const {inputTests}=require('./input.cjs');
const {loginInput}=require('./login-input.cjs');
(async()=>{
 const out=path.resolve('output/windows');fs.mkdirSync(out,{recursive:true});
 const application=await _electron.launch({args:['--enable-unsafe-swiftshader',path.resolve('desktop')],env:{...process.env,STREAMMAX_TEST_PROFILE:path.join(out,'profile')}});
 try{
  const page=await application.firstWindow();let log=[];page.on('console',m=>log.push(m.text()));
  await page.waitForFunction(()=>window.__nvPrimeiroQuadro===1,{},{timeout:180000});
  await page.waitForTimeout(3500);await page.screenshot({path:out+'/aplicativo.png'});
  await loginInput(page,out);
  await inputTests(page,out);
  await page.locator('#canvas').click({position:{x:600,y:550}});
  await page.keyboard.press('ArrowRight');await page.keyboard.press('ArrowLeft');
  fs.writeFileSync(out+'/console.txt',log.join('\n'));
 }finally{await application.close();}
})().catch(e=>{console.error(e);process.exit(1);});
