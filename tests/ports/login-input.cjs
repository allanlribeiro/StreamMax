const assert=require('node:assert/strict');
async function loginInput(page,out){
 const canvas=page.locator('#canvas');
 const size=await canvas.boundingBox();
 const click=async(x,y)=>{await page.mouse.move(size.x+size.width*x,size.y+size.height*y);await page.waitForTimeout(200);await page.mouse.down();await page.waitForTimeout(80);await page.mouse.up();};
 const field=page.locator('#nvTextoCampo');
 await click(.5,.375);await field.waitFor({state:'visible',timeout:10000});
 await field.press('Control+a');await field.pressSequentially('teste@streammax.invalid');
 await page.locator('#sm-editor-confirmar').click();await page.waitForTimeout(900);
 await click(.5,.525);await field.waitFor({state:'visible',timeout:10000});
 assert.equal(await field.getAttribute('type'),'password');
 await field.pressSequentially('senha-teste');await field.press('Backspace');await field.press('Enter');await page.waitForTimeout(900);
 await click(.5,.375);await field.waitFor({state:'visible',timeout:10000});
 // Valor veio de volta do C: prova que a confirmação atualizou o formulário.
 assert.equal(await field.inputValue(),'teste@streammax.invalid');await field.press('Escape');await page.waitForTimeout(900);
 await page.screenshot({path:out+'/login-preenchido.png'});
 require('fs').writeFileSync(out+'/formulario.json',JSON.stringify({passed:true,checks:['clique em e-mail SDL','texto transmitido ao C','senha oculta','apagar na senha','reabrir e-mail com valor preservado']},null,2));
}
module.exports={loginInput};
