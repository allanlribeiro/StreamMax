'use strict';
const {contextBridge,ipcRenderer} = require('electron');
const player = {
  state: {}, active:false,
  open:(url,headers) => {player.active=true; player.state={open:true};ipcRenderer.send('player-open',url,headers);},
  close:() => {player.active=false;ipcRenderer.send('player-close');},
  command:(name,value) => ipcRenderer.send('player-command',name,value),
  onState: callback => {ipcRenderer.on('player-state',(_,s)=>callback(s));},
  onClose: callback => {ipcRenderer.on('player-closed',(_,s)=>callback(s));}
};
contextBridge.exposeInMainWorld('streammaxDesktop', {
 open:player.open,close:player.close,command:player.command,onState:player.onState,onClose:player.onClose
});
