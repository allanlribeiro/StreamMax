const { contextBridge, ipcRenderer } = require('electron');
contextBridge.exposeInMainWorld('streammaxDesktop', Object.freeze({
  platform: 'windows', prototype: true,
  authStatus: () => ipcRenderer.invoke('streammax:auth-status'),
  authStart: () => ipcRenderer.invoke('streammax:auth-start'),
  authPoll: () => ipcRenderer.invoke('streammax:auth-poll'),
  authCancel: () => ipcRenderer.invoke('streammax:auth-cancel'),
  openLocalVideo: () => ipcRenderer.invoke('streammax:open-video')
}));
