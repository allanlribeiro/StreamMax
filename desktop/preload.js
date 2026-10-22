const { contextBridge, ipcRenderer } = require('electron');
contextBridge.exposeInMainWorld('streammaxDesktop', Object.freeze({
  platform: 'windows', prototype: true,
  authStatus: () => ipcRenderer.invoke('streammax:auth-status'),
  authRestore: () => ipcRenderer.invoke('streammax:auth-restore'),
  authSignOut: () => ipcRenderer.invoke('streammax:auth-signout'),
  reloadCatalogs: () => ipcRenderer.invoke('streammax:catalogs-reload'),
  catalogPage: (key,skip=0,extras={}) => ipcRenderer.invoke('streammax:catalog-page',key,skip,extras),
  authStart: () => ipcRenderer.invoke('streammax:auth-start'),
  authPoll: () => ipcRenderer.invoke('streammax:auth-poll'),
  authCancel: () => ipcRenderer.invoke('streammax:auth-cancel'),
  openLocalVideo: () => ipcRenderer.invoke('streammax:open-video')
}));
