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
  openLoginPage: () => ipcRenderer.invoke('streammax:open-login'),
  searchSources: (type,id,requestId) => ipcRenderer.invoke('streammax:source-search',type,id,requestId),
  onSourcesProgress: callback => {
    if(typeof callback!=='function')throw new TypeError('Callback required');
    const listener=(_event,payload)=>callback(payload);
    ipcRenderer.on('streammax:sources-progress',listener);
    return ()=>ipcRenderer.removeListener('streammax:sources-progress',listener);
  },
  openLocalVideo: () => ipcRenderer.invoke('streammax:open-video')
}));
