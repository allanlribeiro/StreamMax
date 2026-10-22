const { app, BrowserWindow, dialog, ipcMain, shell, safeStorage } = require('electron');
const path = require('path');
const fs = require('fs');
const { pathToFileURL } = require('url');
const {createAuthService}=require('./auth-service');
const {createSessionStore}=require('./session-store');
const {createSessionClient}=require('./session-client');
const {createCatalogClient}=require('./catalog-client');
const {createSourceDiscovery}=require('./source-discovery');
const {createMpvPlayer}=require('./mpv-player');
const {createWatchProgressStore}=require('./watch-progress-store');
const {createPlaybackCoordinator}=require('./playback-coordinator');
const {createRemoteProgressClient}=require('./remote-progress-client');
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
const remoteProgress=sessionClient?createRemoteProgressClient({
 sessionClient,endpoint:process.env.STREAMMAX_SUPABASE_URL,
 publicKey:process.env.STREAMMAX_SUPABASE_PUBLIC_KEY,profileId:1
}):null;
const sourceDiscovery=createSourceDiscovery();
const mpvSettingsFile=path.join(app.getPath('userData'),'mpv-settings.json');
function savedMpvPath(){
 try{
  const saved=JSON.parse(fs.readFileSync(mpvSettingsFile,'utf8'));
  return typeof saved.executable==='string'?saved.executable:null;
 }catch{return null;}
}
const mpvPlayer=createMpvPlayer({executable:savedMpvPath()||process.env.STREAMMAX_MPV_PATH||null});
const watchProgressStore=createWatchProgressStore({
 safeStorage,filePath:path.join(app.getPath('userData'),'watch-progress.enc')
});
const playback=createPlaybackCoordinator({
 player:mpvPlayer,sources:sourceDiscovery,store:watchProgressStore,
 remoteResume:remoteProgress?(contentId,videoId)=>remoteProgress.resume(contentId,videoId):null,
 onProgress:position=>{
  if(window&&!window.isDestroyed())window.webContents.send('streammax:player-progress',position);
 },
 onError:message=>{
  if(window&&!window.isDestroyed())window.webContents.send('streammax:player-warning',{message});
 }
});
mpvPlayer.onStatus(event=>{
 if(window&&!window.isDestroyed())window.webContents.send('streammax:player-status',event);
});
mpvPlayer.onTracks(event=>{
 if(window&&!window.isDestroyed())window.webContents.send('streammax:player-tracks',event);
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
let remoteWarmup=null;
function warmRemoteHistory(){
 if(!remoteProgress||remoteWarmup)return;
 // Fetch in parallel with UI catalog loading. Failure never blocks login.
 const pending=remoteProgress.list().catch(()=>{}).finally(()=>{
  if(remoteWarmup===pending)remoteWarmup=null;
 });
 remoteWarmup=pending;
}
ipcMain.handle('streammax:auth-status',()=>authService.status());
ipcMain.handle('streammax:auth-restore',async()=>{
 const result=await authService.restore();
 if(result.authenticated)warmRemoteHistory();
 return result;
});
ipcMain.handle('streammax:auth-signout',async()=>{
 catalogClient?.clear();
 sourceDiscovery.reset();
 remoteProgress?.clear();
 await playback.signOut();
 return authService.signOut();
});
ipcMain.handle('streammax:auth-start',()=>authService.start());
ipcMain.handle('streammax:auth-poll',async()=>{
 const result=await authService.poll();
 if(result.authenticated)warmRemoteHistory();
 return result;
});
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
ipcMain.handle('streammax:metadata',(_event,key,type,id)=>{
 if(!catalogClient)throw new Error('Catalog provider unavailable');
 if(typeof key!=='string'||key.length>128)throw new TypeError('Invalid catalog key');
 return catalogClient.meta(key,type,id);
});
ipcMain.handle('streammax:player-availability',()=>mpvPlayer.availability());
ipcMain.handle('streammax:player-select',async()=>{
 const result=await dialog.showOpenDialog(window,{
  title:'Selecione mpv.exe',properties:['openFile'],
  filters:[{name:'mpv para Windows',extensions:['exe']}]
 });
 if(result.canceled||!result.filePaths.length)return mpvPlayer.availability();
 const exe=result.filePaths[0];
 const changed=mpvPlayer.setExecutable(exe);
 const tmp=mpvSettingsFile+'.tmp';
 fs.mkdirSync(path.dirname(mpvSettingsFile),{recursive:true});
 try{
  fs.writeFileSync(tmp,JSON.stringify({executable:exe}),{encoding:'utf8',mode:0o600});
  fs.renameSync(tmp,mpvSettingsFile);
 }catch(error){try{fs.rmSync(tmp,{force:true});}catch{}throw error;}
 return changed;
});
ipcMain.handle('streammax:player-play',(_event,sourceId,metadata)=>{
 if(typeof sourceId!=='string'||!/^[0-9a-f]{32}$/.test(sourceId))
  throw new TypeError('Identificador da fonte inválido');
 return playback.play(sourceId,metadata);
});
ipcMain.handle('streammax:progress-list',()=>playback.list());
ipcMain.handle('streammax:progress-remote',()=>{
 if(!remoteProgress)throw new Error('Servidor da conta não configurado');
 return remoteProgress.list();
});
ipcMain.handle('streammax:player-progress',()=>playback.currentProgress());
ipcMain.handle('streammax:player-command',(_event,command)=>mpvPlayer.command(command));
ipcMain.handle('streammax:player-track',(_event,type,id)=>mpvPlayer.selectTrack(type,id));
ipcMain.handle('streammax:player-stop',()=>playback.stop());
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
let shutdownReady=false;
let shutdownInProgress=false;
app.on('before-quit',event=>{
 if(shutdownReady)return;
 event.preventDefault();
 if(shutdownInProgress)return;
 shutdownInProgress=true;
 // Flush the current watch position before Electron terminates.
 Promise.resolve(playback.stop()).catch(()=>{}).finally(()=>{
  shutdownReady=true;
  app.quit();
 });
});
app.on('window-all-closed',()=>{if(process.platform!=='darwin')app.quit();});
