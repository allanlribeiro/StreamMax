'use strict';
const net = require('node:net');
const crypto = require('node:crypto');
const {spawn} = require('node:child_process');
const {EventEmitter} = require('node:events');
function urlHttp(value) {
  if (typeof value !== 'string' || value.length > 32768) throw Error('Endereço inválido');
  const u = new URL(value);
  if (!['http:', 'https:'].includes(u.protocol)) throw Error('Use um endereço HTTP ou HTTPS');
  return value;
}
function headers(text) {
  return String(text || '').split(/\r?\n/).filter(Boolean).map(line => {
    if (!/^[\w-]+:[^\r\n\x00]*$/.test(line)) throw Error('Cabeçalho inválido');
    return line;
  });
}
class Player extends EventEmitter {
  constructor(executable, options={}) { super(); this.options=options; this.executable = executable; this.serial = 0; this.reset(); }
  reset() { this.state = {pos:0,duration:0,playing:false,ready:false,open:false,width:0,height:0,error:false,ended:false,tracks:[],message:''}; }
  publish() { this.emit('state', {...this.state}); }
  async open(url, cabecalhos = '') {
    urlHttp(url); const hs = headers(cabecalhos); this.close(); this.reset();
    const generation = ++this.serial;
    const pipe = '\\\\.\\pipe\\streammax-' + crypto.randomUUID();
    this.state.open = true; this.publish();
    const child = spawn(this.executable, ['--no-config','--idle=yes','--force-window=yes','--keep-open=no','--terminal=no','--ytdl=no','--title=StreamMax','--alang=pt-BR,por,pt','--slang=pt-BR,por,pt','--hwdec=auto-safe','--input-ipc-server='+pipe,...(this.options.logFile?['--log-file='+this.options.logFile]:[])], {windowsHide:true, stdio:this.options.diagnostics?['ignore','pipe','pipe']:'ignore'});
    this.child = child;
    if(this.options.diagnostics){for(const stream of [child.stdout,child.stderr])if(stream)stream.on('data',data=>this.emit('diagnostic',data.toString()));}
    child.once('error', () => this.fail(generation, 'Não foi possível iniciar o player.'));
    child.once('exit', (code,signal) => {
      if (this.serial !== generation) return;
      if(code && !this.state.ended){this.state.error=true;this.state.message='O player encerrou com código '+code;}
      this.emit('diagnostic','Saída do player: '+code+' '+(signal||''));
      this.state.playing=false; this.state.open=false; this.publish();
      this.emit('closed', {ended:this.state.ended});
    });
    try {
      const socket = await new Promise((resolve,reject) => {
        const start = Date.now();
        const connect = () => {
          if (generation !== this.serial) return reject(Error('Cancelado'));
          const s = net.connect(pipe);
          s.once('connect', () => {s.removeAllListeners('error'); resolve(s);});
          s.once('error', () => {s.destroy(); if(Date.now()-start>10000) reject(Error('Tempo esgotado')); else setTimeout(connect,100);});
        }; connect();
      });
      if (generation !== this.serial) return socket.destroy();
      this.socket = socket; let buffer = '';
      socket.on('error', () => this.fail(generation,'O player perdeu a conexão.'));
      socket.on('data', chunk => {
        buffer += chunk.toString();
        if (buffer.length > 2e6) { socket.destroy(); return; }
        for(let at; (at=buffer.indexOf('\n'))>=0;) {
          const line = buffer.slice(0,at); buffer=buffer.slice(at+1);
          try { this.message(JSON.parse(line),generation); } catch (_) {}
        }
      });
      ['time-pos','duration','pause','width','height','track-list','eof-reached'].forEach((p,i) => this.send(['observe_property',i+1,p]));
      this.send(['set_property','http-header-fields',hs]);
      this.send(['loadfile',url,'replace']);
    } catch (_) { if(!this.state.error)this.fail(generation,'Não foi possível conectar ao player.'); }
  }
  fail(generation, message) { if(generation!==this.serial) return; this.state.error=true;this.state.message=message;this.publish(); }
  message(m,generation) {
    if(generation!==this.serial) return;
    if(m.event==='file-loaded') {this.state.ready=true;this.state.playing=true;}
    if(m.event==='end-file') {
      if(m.reason==='eof') this.state.ended=true;
      else if(m.reason==='error') {this.state.error=true;this.state.message='Não foi possível reproduzir esta fonte.';}
      this.state.playing=false;
    }
    if(m.event==='property-change') {
      const map={'time-pos':'pos',duration:'duration',width:'width',height:'height','track-list':'tracks','eof-reached':'ended'};
      if(m.name==='pause') this.state.playing=!m.data;
      else if(map[m.name] && m.data!=null) this.state[map[m.name]]=m.data;
    }
    this.publish();
  }
  send(command) { if(this.socket && !this.socket.destroyed) this.socket.write(JSON.stringify({command})+'\n'); }
  command(name,value) {
    if(name==='pause') this.send(['set_property','pause',!!value]);
    else if(name==='seek' && Number.isFinite(value)) this.send(['seek',Math.max(0,value),'absolute']);
    else if(name==='volume' && Number.isFinite(value)) this.send(['set_property','volume',Math.max(0,Math.min(100,value))]);
    else if(name==='audio') this.send(['set_property','aid',value]);
    else if(name==='subtitle') this.send(['set_property','sid',value]);
    else if(name==='subtitle-url') this.send(['sub-add',urlHttp(value),'select']);
    else if(name==='speed' && Number.isFinite(value)) this.send(['set_property','speed',Math.max(.25,Math.min(4,value))]);
  }
  close() { this.serial++; if(this.socket) this.socket.destroy(); this.socket=null; if(this.child) this.child.kill(); this.child=null; }
}
module.exports = {Player,urlHttp,headers};
