const { contextBridge } = require('electron');
contextBridge.exposeInMainWorld('streammaxDesktop', Object.freeze({ platform: 'windows', prototype: true }));
