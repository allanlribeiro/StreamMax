const { app, BrowserWindow, dialog, ipcMain, shell, safeStorage } = require('electron');
const path = require('path');
const fs = require('fs');
const { pathToFileURL } = require('url');
const {createAuthService}=require('./auth-service');
const {createSessionStore}=require('./session-store');
const sessionStore=createSessionStore({safeStorage,filePath:path.join(app.getPath('userData'),'session.enc')});
const authService=createAuthService({
 endpoint:process.env.STREAMMAX_SUPABASE_URL,
 publicKey:process.env.STREAMMAX_SUPABASE_PUBLIC_KEY,
 redirectUrl:process.env.STREAMMAX_LOGIN_REDIRECT_URL,
 sessionStore
});
app.setAppUserModelId('com.streammax.desktop');
let window;
function createWindow() {
  const options = {
    width: 1280, height: 800, minWidth: 900, minHeight: 560,
    title: 'StreamMax', backgroundColor: '#080e1b',
    webPreferences: { preload: path.join(__dirname, 'preload.js'), contextIsolation: true, nodeIntegration: false, sandbox: true }
  };
  const ico = path.join(__dirname, 'assets', 'streammax.ico');
  if (fs.existsSync(ico)) options.icon = ico;
  window = new BrowserWindow(options);
  window.removeMenu();
  window.loadFile('index.html');
  window.webContents.on('before-input-event', (event,input) => {
    if(input.type==='keyDown' && input.key==='F11') {window.setFullScreen(!window.isFullScreen());event.preventDefault();}
  });
  window.webContents.setWindowOpenHandler(({url}) => {
    try {if(new URL(url).protocol==='https:') shell.openExternal(url);} catch {}
    return {action:'deny'};
  });
  window.webContents.on('will-navigate', event => event.preventDefault());
}
ipcMain.handle('streammax:open-video', async () => {
  const response = await dialog.showOpenDialog(window, {
    title:'Abrir vídeo local', properties:['openFile'],
    filters:[{name:'Vídeos',extensions:['mp4','m4v','webm','mov','mkv']}]});
  return response.canceled ? null : pathToFileURL(response.filePaths[0]).href;
});
ipcMain.handle('streammax:auth-status',()=>authService.status());
ipcMain.handle('streammax:auth-start',()=>authService.start());
ipcMain.handle('streammax:auth-poll',()=>authService.poll());
ipcMain.handle('streammax:auth-cancel',()=>authService.cancel());
app.whenReady().then(()=>{createWindow();app.on('activate',()=>{if(BrowserWindow.getAllWindows().length===0)createWindow();});});
app.on('window-all-closed',()=>{if(process.platform!=='darwin')app.quit();});
