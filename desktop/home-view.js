'use strict';
(function(root, exportTarget){
  // Data-to-DOM renderer: no placeholder titles, no unsafe HTML from addons.
  function safePosterUrl(value){
    if(typeof value!=='string'||value.length>2048)return '';
    try {
      const url=new URL(value);
      return url.protocol==='https:'&&!url.username&&!url.password?url.href:'';
    } catch {return '';}
  }
  function normalizeItem(item){
    if(!item||typeof item!=='object')return null;
    if(typeof item.id!=='string'||!item.id||typeof item.name!=='string'||!item.name)return null;
    return {
      id:item.id,type:String(item.type||'movie'),name:item.name,
      poster:safePosterUrl(item.poster),
      background:safePosterUrl(item.background)||safePosterUrl(item.landscapePoster)||safePosterUrl(item.poster),
      logo:safePosterUrl(item.logo),description:typeof item.description==='string'?item.description:'',
      releaseInfo:typeof item.releaseInfo==='string'?item.releaseInfo:''
    };
  }
  function createEl(tag,className,textContent){
    const node=document.createElement(tag);
    if(className)node.className=className;
    if(textContent!==undefined)node.textContent=textContent;
    return node;
  }
  function createHomeView({root:container,onOpenDetails=()=>{}}){
    if(!container)throw new TypeError('Home container required');
    const hero=createEl('section','hero');
    const heroImage=createEl('img','hero__art');
    heroImage.alt='';heroImage.decoding='async';heroImage.hidden=true;
    const shade=createEl('div','hero__shade');
    const metadata=createEl('div','hero__info');
    const title=createEl('h1','hero__title','');
    const desc=createEl('p','hero__description','');
    const action=createEl('button','hero__details','Ver detalhes');
    action.type='button';action.hidden=true;
    metadata.append(title,desc,action);
    hero.append(heroImage,shade,metadata);
    const rows=createEl('div','catalog-rows');
    container.replaceChildren(hero,rows);
    const all=[];
    let active=null;
    function select(item){
      if(!item)return;
      active=item;
      heroImage.hidden=!item.background;
      if(item.background&&heroImage.getAttribute('src')!==item.background)heroImage.src=item.background;
      else heroImage.removeAttribute('src');
      title.textContent=item.name;
      desc.textContent=[item.releaseInfo,item.description].filter(Boolean).join(' • ');
      action.hidden=false;
    }
    action.addEventListener('click',()=>{if(active)onOpenDetails(active);});
    function addRow(data){
      if(!data||typeof data!=='object')return;
      const label=String(data.title||data.catalogName||'').trim();
      const items=(Array.isArray(data.items)?data.items:[]).map(normalizeItem).filter(Boolean);
      if(!items.length)return;
      const section=createEl('section','catalog-row');
      section.dataset.catalogKey=String(data.key||'');
      const heading=createEl('h2','catalog-row__title',label);
      const rail=createEl('div','catalog-row__rail');
      rail.setAttribute('aria-label',label);
      for(const item of items){
        const card=createEl('button','poster-card');
        card.type='button';card.title=item.name;card.setAttribute('aria-label',item.name);
        const image=createEl('img','poster-card__image');
        if(item.poster){image.src=item.poster;image.alt='';image.loading='lazy';}
        else {image.hidden=true;card.classList.add('poster-card--noart');}
        const caption=createEl('span','poster-card__title',item.name);
        card.append(image,caption);
        card.addEventListener('mouseenter',()=>select(item));
        card.addEventListener('focus',()=>select(item));
        card.addEventListener('click',()=>onOpenDetails(item));
        card.addEventListener('keydown',event=>{
          let next=null;
          if(event.key==='ArrowLeft'||event.key==='ArrowRight'){
            next=event.key==='ArrowRight'?card.nextElementSibling:card.previousElementSibling;
          }else if(event.key==='ArrowUp'||event.key==='ArrowDown'){
            const otherRow=event.key==='ArrowDown'?section.nextElementSibling:section.previousElementSibling;
            const rowCards=otherRow?Array.from(otherRow.querySelectorAll('.poster-card')):[];
            const currentIndex=Array.from(rail.children).indexOf(card);
            next=rowCards[Math.min(currentIndex,rowCards.length-1)]||null;
          }
          if(next&&next.tagName==='BUTTON'){
            event.preventDefault();next.focus();next.scrollIntoView({block:'nearest',inline:'nearest'});
          }
        });
        rail.append(card);
        all.push(item);
      }
      // Keep vertical scroll for ordinary wheel; Shift+wheel navigates the rail.
      rail.addEventListener('wheel',event=>{
        if(event.shiftKey&&rail.scrollWidth>rail.clientWidth){
          event.preventDefault();rail.scrollLeft+=event.deltaY||event.deltaX;
        }
      },{passive:false});
      section.append(heading,rail);
      rows.append(section);
      if(!active)select(items[0]);
    }
    function clear(){
      rows.replaceChildren();
      all.length=0;
      active=null;
      heroImage.hidden=true;heroImage.removeAttribute('src');
      title.textContent='';desc.textContent='';action.hidden=true;
    }
    function hasContent(){return all.length>0;}
    return {addRow,clear,hasContent};
  }
  const api={createHomeView,safePosterUrl,normalizeItem};
  if(typeof module==='object'&&module.exports)module.exports=api;
  if(root)root.StreamMaxHomeView=api;
})(typeof window!=='undefined'?window:null);
