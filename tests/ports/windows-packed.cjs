const {_electron}=require('playwright');
const assert=require('node:assert/strict');
const path=require('node:path');
const fs=require('node:fs');
(async()=>{
 const dir=path.resolve('output/windows');
 const executable=path.resolve('desktop/dist/win-unpacked/StreamMax.exe');
 for(const resource of ['web/index.wasm','web/index.data','web/fontes.data','mpv/mpv.exe'])assert.ok(fs.existsSync(path.resolve('desktop/dist/win-unpacked/resources',resource)),resource);
 const app=await _electron.launch({executablePath:executable,args:['--enable-unsafe-swiftshader'],env:{...process.env,STREAMMAX_TEST_PROFILE:path.join(dir,'packaged-profile')}});
 try{
  assert.equal(await app.evaluate(({app})=>app.isPackaged),true);
  const page=await app.firstWindow();await page.waitForFunction(()=>window.__nvPrimeiroQuadro===1,null,{timeout:180000});
  await page.waitForTimeout(1500);await page.screenshot({path:dir+'/windows-empacotado.png'});
  fs.writeFileSync(dir+'/empacotamento.json',JSON.stringify({passed:true,packaged:true,checks:['executável Windows','ASAR','recursos WASM','player incluído','primeiro quadro']},null,2));
 }finally{await app.close();}
})().catch(e=>{console.error(e);process.exit(1);});
