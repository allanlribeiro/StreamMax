const { contextBridge, ipcRenderer } = require('electron');
contextBridge.exposeInMainWorld('streammaxDesktop', Object.freeze({
  platform: 'windows', prototype: true,
  openLocalVideo: () => ipcRenderer.invoke('streammax:open-video')
}));
