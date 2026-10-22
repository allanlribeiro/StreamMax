const { app, BrowserWindow, dialog, ipcMain, shell, safeStorage } = require('electron');
const path = require('path');
const fs = require('fs');
const { pathToFileURL } = require('url');
const {createAuthService}=require('./auth-service');
const {createSessionStore}=require('./session-store');
const {createSessionClient}=require('./session-client');
const {createCatalogClient}=require('./catalog-client');
const {createSourceDiscovery}=require('./source-discovery');
const sessionStore=createSessionStore({safeStorage,filePath:path.join(app.getPath('userData'),'session.enc')});
const sessionClient=(process.env.STREAMMAX_SUPABASE_URL&&process.env.STREAMMAX_SUPABASE_PUBLIC_KEY)
  ?createSessionClient({
    endpoint:process.env.STREAMMAX_SUPABASE_URL,
    publicKey:process.env.STREAMMAX_SUPABASE_PUBLIC_KEY,
    store:sessionStore
  }):null;
const authService=createAuthService({
 endpoint:process.env.STREAMMAX_SUPABASE_URL,
 publicKey:process.env.STREAMMAX_SUPABASE_PUBLIC_KEY,
 redirectUrl:process.env.STREAMMAX_LOGIN_REDIRECT_URL,
 sessionStore,
 sessionClient
});
const catalogClient=sessionClient?createCatalogClient({
  sessionClient,
  endpoint:process.env.STREAMMAX_SUPABASE_URL,
  publicKey:process.env.STREAMMAX_SUPABASE_PUBLIC_KEY,
  profileId:1
}):null;
const sourceDiscovery=createSourceDiscovery();
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
ipcMain.handle('streammax:auth-restore',()=>authService.restore());
ipcMain.handle('streammax:auth-signout',()=>{catalogClient?.clear();sourceDiscovery.reset();return authService.signOut();});
ipcMain.handle('streammax:auth-start',()=>authService.start());
ipcMain.handle('streammax:auth-poll',()=>authService.poll());
ipcMain.handle('streammax:auth-cancel',()=>authService.cancel());
ipcMain.handle('streammax:open-login',async()=>{
 const target=authService.verificationPage();
 if(!target)throw new Error('No current authorization page');
 const url=new URL(target);
 if(url.protocol!=='https:'||url.username||url.password)throw new Error('Invalid authorization URL');
 await shell.openExternal(url.href);
 return true;
});
ipcMain.handle('streammax:catalogs-reload',()=>{
  if(!catalogClient)throw new Error('Account service not configured');
  return catalogClient.reload();
});
ipcMain.handle('streammax:catalog-page',(_event,key,skip=0,extras={})=>{
  if(!catalogClient)throw new Error('Account service not configured');
  if(typeof key!=='string'||key.length>128)throw new TypeError('Invalid catalog key');
  return catalogClient.page(key,skip,extras);
});
ipcMain.handle('streammax:source-search',(_event,type,id,requestId)=>{
 if(!catalogClient)throw new Error('Catalog provider unavailable');
 if(typeof requestId!=='string'||requestId.length>72)throw new TypeError('Invalid request identifier');
 return sourceDiscovery.search({
  type,id,providers:catalogClient.getSourceProviders(),
  onUpdate:state=>{
   if(window&&!window.isDestroyed())window.webContents.send('streammax:sources-progress',{requestId,...state});
  }
 });
});
app.whenReady().then(()=>{createWindow();app.on('activate',()=>{if(BrowserWindow.getAllWindows().length===0)createWindow();});});
app.on('window-all-closed',()=>{if(process.platform!=='darwin')app.quit();});
