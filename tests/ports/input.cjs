const assert=require('node:assert/strict');
const fs=require('node:fs');
async function inputTests(page,output){
 await page.evaluate(()=>{nvTexto.configurar(0,128);nvTexto.abrir('');});
 const field=page.locator('#nvTextoCampo');await field.waitFor({state:'visible'});
 await field.pressSequentially('StreamMax');await field.press('Backspace');
 assert.equal(await field.inputValue(),'StreamMa');
 await field.press('Control+Home');await field.press('Delete');assert.equal(await field.inputValue(),'treamMa');
 await field.press('Control+a');await page.keyboard.insertText('São Paulo — ação');
 assert.equal(await field.inputValue(),'São Paulo — ação');
 await field.click();await field.press('Home');await field.press('Shift+ArrowRight');await field.press('Shift+ArrowRight');await field.press('Shift+ArrowRight');
 assert.equal(await field.evaluate(el=>el.value.slice(el.selectionStart,el.selectionEnd)),'São');
 await page.keyboard.insertText('Olá');assert.equal(await field.inputValue(),'Olá Paulo — ação');
 // Seleção com arraste do mouse, sem definir selectionStart por script.
 const box=await field.boundingBox();await page.mouse.move(box.x+18,box.y+box.height/2);await page.mouse.down();await page.mouse.move(box.x+150,box.y+box.height/2,{steps:15});await page.mouse.up();
 assert.ok(await field.evaluate(el=>el.selectionEnd>el.selectionStart),'mouse seleciona texto');
 await page.screenshot({path:output+'/edicao-mouse.png'});
 await page.locator('#sm-editor-confirmar').click();
 assert.equal(await page.evaluate(()=>nvTexto.fim),1);
 assert.equal(await page.evaluate(()=>nvTexto.valor),'Olá Paulo — ação');
 await page.evaluate(()=>{nvTexto.configurar(2,6);nvTexto.abrir('');});
 assert.equal(await field.getAttribute('type'),'password');await field.pressSequentially('123456789');assert.equal(await field.inputValue(),'123456');
 await field.press('Escape');assert.equal(await page.evaluate(()=>nvTexto.fim),0);
 await page.evaluate(()=>{nvTexto.configurar(1,256);nvTexto.abrir('teste@example.com');});
 await field.press('Control+a');await page.keyboard.insertText('usuario@example.com');await field.press('Enter');
 assert.equal(await page.evaluate(()=>nvTexto.valor),'usuario@example.com');assert.equal(await page.evaluate(()=>nvTexto.fim),1);
 await page.evaluate(()=>{nvTexto.configurar(0,128);nvTexto.abrir('apagar');});
 await field.press('Control+a');await field.press('Backspace');await page.locator('#sm-editor-confirmar').click();
 assert.equal(await page.evaluate(()=>nvTexto.valor),'');assert.equal(await page.evaluate(()=>nvTexto.fim),1);
 const result={passed:true,checks:['digitação','acentos','Backspace','Delete','Ctrl+A','seleção e substituição','seleção com mouse','confirmar com mouse','senha oculta','limite de caracteres','cancelar','e-mail','campo vazio']};
 fs.writeFileSync(output+'/entrada.json',JSON.stringify(result,null,2));
}
module.exports={inputTests};
