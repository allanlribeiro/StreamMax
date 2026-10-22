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
  loadMetadata: (key,type,id) => ipcRenderer.invoke('streammax:metadata',key,type,id),
  searchSources: (type,id,requestId) => ipcRenderer.invoke('streammax:source-search',type,id,requestId),
  onSourcesProgress: callback => {
    if(typeof callback!=='function')throw new TypeError('Callback required');
    const listener=(_event,payload)=>callback(payload);
    ipcRenderer.on('streammax:sources-progress',listener);
    return ()=>ipcRenderer.removeListener('streammax:sources-progress',listener);
  },
  playerAvailability: () => ipcRenderer.invoke('streammax:player-availability'),
  selectMpv: () => ipcRenderer.invoke('streammax:player-select'),
  playSource: (id,metadata) => ipcRenderer.invoke('streammax:player-play',id,metadata),
  listWatchProgress: () => ipcRenderer.invoke('streammax:progress-list'),
  playerProgress: () => ipcRenderer.invoke('streammax:player-progress'),
  playerCommand: command => ipcRenderer.invoke('streammax:player-command',command),
  playerTrack: (type,id) => ipcRenderer.invoke('streammax:player-track',type,id),
  stopPlayer: () => ipcRenderer.invoke('streammax:player-stop'),
  onPlayerProgress: callback => {
    if(typeof callback!=='function')throw new TypeError('Callback required');
    const listener=(_event,payload)=>callback(payload);
    ipcRenderer.on('streammax:player-progress',listener);
    return ()=>ipcRenderer.removeListener('streammax:player-progress',listener);
  },
  onPlayerWarning: callback => {
    if(typeof callback!=='function')throw new TypeError('Callback required');
    const listener=(_event,payload)=>callback(payload);
    ipcRenderer.on('streammax:player-warning',listener);
    return ()=>ipcRenderer.removeListener('streammax:player-warning',listener);
  },
  onPlayerTracks: callback => {
    if(typeof callback!=='function')throw new TypeError('Callback required');
    const listener=(_event,payload)=>callback(payload);
    ipcRenderer.on('streammax:player-tracks',listener);
    return ()=>ipcRenderer.removeListener('streammax:player-tracks',listener);
  },
  onPlayerStatus: callback => {
    if(typeof callback!=='function')throw new TypeError('Callback required');
    const listener=(_event,payload)=>callback(payload);
    ipcRenderer.on('streammax:player-status',listener);
    return ()=>ipcRenderer.removeListener('streammax:player-status',listener);
  },
  openLocalVideo: () => ipcRenderer.invoke('streammax:open-video')
}));
