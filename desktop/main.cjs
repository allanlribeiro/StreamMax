'use strict';
const {app,BrowserWindow,ipcMain,shell,session,dialog} = require('electron');
const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const {Player} = require('./mpv.cjs');
let win,server,origin,player;
const web = app.isPackaged ? path.join(process.resourcesPath,'web') : path.join(__dirname,'web');
const mpv = app.isPackaged ? path.join(process.resourcesPath,'mpv','mpv.exe') : path.join(__dirname,'mpv','mpv.exe');
const mime = {'.html':'text/html; charset=utf-8','.js':'application/javascript','.wasm':'application/wasm','.png':'image/png','.data':'application/octet-stream','.json':'application/json'};
function trusted(event) { return !!win && event.sender===win.webContents && event.senderFrame===win.webContents.mainFrame && event.senderFrame.url.startsWith(origin+'/'); }
app.setName('StreamMax');
if(process.env.STREAMMAX_TEST_PROFILE)app.setPath('userData',process.env.STREAMMAX_TEST_PROFILE);
if(!app.requestSingleInstanceLock())app.quit();
app.whenReady().then(async () => {
 if(process.platform==='win32')app.setAppUserModelId('com.streammax.desktop');
 server=http.createServer((req,res)=>{
   if(req.headers.host!==new URL(origin).host) {res.writeHead(403);return res.end();}
   if(!['GET','HEAD'].includes(req.method)){res.writeHead(405);return res.end();}
   let name;try{name=decodeURIComponent(new URL(req.url,origin).pathname);}catch(_){res.writeHead(400);return res.end();}
   const file=path.resolve(web,'.'+(name==='/'?'/index.html':name));
   if(!file.startsWith(web+path.sep)) {res.writeHead(403);return res.end();}
   res.setHeader('Cross-Origin-Opener-Policy','same-origin');res.setHeader('Cross-Origin-Embedder-Policy','require-corp');
   res.setHeader('Cross-Origin-Resource-Policy','cross-origin');res.setHeader('X-Content-Type-Options','nosniff');
   res.setHeader('Content-Type',mime[path.extname(file)]||'application/octet-stream');
   const stream=fs.createReadStream(file);stream.on('error',()=>{res.writeHead(404);res.end();});stream.pipe(res);
 });
 const portFile=path.join(app.getPath('userData'),'origin-port.txt');
 let savedPort=0;
 try {savedPort=Number(fs.readFileSync(portFile,'utf8'));}catch(_){}
 if(!Number.isInteger(savedPort)||savedPort<1024||savedPort>65535)savedPort=0;
 try {
   await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(savedPort,'127.0.0.1',resolve);});
 }catch(_){dialog.showErrorBox('StreamMax','A porta local está ocupada. Feche outra instância do StreamMax e tente novamente.');app.quit();return;}
 origin='http://127.0.0.1:'+server.address().port;
 fs.mkdirSync(path.dirname(portFile),{recursive:true});fs.writeFileSync(portFile,String(server.address().port));
 session.defaultSession.setPermissionRequestHandler((_,__,cb)=>cb(false));
 session.defaultSession.webRequest.onHeadersReceived((details,callback)=>{
   const responseHeaders={...details.responseHeaders};
   // APIs de addons são escolhidas pelo usuário. A liberação fica restrita a
   // respostas XHR/media da janela local; não desativa a segurança do Chromium.
   if(details.webContentsId===win?.webContents.id && !details.url.startsWith(origin+'/')) {
     if(['xhr','media','image'].includes(details.resourceType)) {
       for(const key of Object.keys(responseHeaders)) if(/^access-control-allow-origin$|^cross-origin-resource-policy$/i.test(key)) delete responseHeaders[key];
       responseHeaders['Access-Control-Allow-Origin']=[origin];responseHeaders['Access-Control-Allow-Headers']=['authorization,content-type,apikey,x-client-info,range'];responseHeaders['Access-Control-Allow-Methods']=['GET,POST,PUT,PATCH,DELETE,OPTIONS'];responseHeaders['Cross-Origin-Resource-Policy']=['cross-origin'];
     }
   }
   callback({responseHeaders});
 });
 win=new BrowserWindow({width:1440,height:810,minWidth:960,minHeight:540,title:'StreamMax',icon:path.join(__dirname,'icon.ico'),backgroundColor:'#0e0f12',autoHideMenuBar:true,webPreferences:{preload:path.join(__dirname,'preload.cjs'),contextIsolation:true,nodeIntegration:false,sandbox:true}});
 win.webContents.setWindowOpenHandler(({url})=>{if(/^https:\/\//.test(url))shell.openExternal(url);return{action:'deny'};});
 win.webContents.on('will-navigate',(ev,url)=>{if(!url.startsWith(origin+'/'))ev.preventDefault();});
 win.webContents.on('before-input-event',(ev,input)=>{if(input.type==='keyDown'&&input.key==='F11'){ev.preventDefault();win.setFullScreen(!win.isFullScreen());}});
 player=new Player(mpv);
 player.on('state',s=>{if(!win.isDestroyed())win.webContents.send('player-state',s);});
 player.on('closed',s=>{if(!win.isDestroyed()){win.show();win.focus();win.webContents.send('player-closed',s);}});
 ipcMain.on('player-open',(ev,url,headers)=>{if(trusted(ev))player.open(url,headers).catch(()=>{});});
 ipcMain.on('player-close',ev=>{if(trusted(ev))player.close();});
 ipcMain.on('player-command',(ev,name,value)=>{if(trusted(ev))player.command(name,value);});
 await win.loadURL(origin+'/');
});
app.on('second-instance',()=>{if(win){if(win.isMinimized())win.restore();win.show();win.focus();}});
app.on('window-all-closed',()=>app.quit());
app.on('before-quit',()=>{if(player)player.close();if(server)server.close();});
