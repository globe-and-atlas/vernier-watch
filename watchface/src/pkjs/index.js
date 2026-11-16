var Clay=require('@rebble/clay');
var defaults={Palette:0,Background:0,TextColor:0xFFFFFF,HourColor:0x55FFFF,MinuteColor:0xFFAA55,TimeFormat:0,DateFormat:0,Labels:1,DigitalTime:1};
function integer(value,fallback,max){
 if(value===undefined || value===null || value==='' || typeof value==='boolean')return fallback;
 var number=Number(value);
 return isFinite(number) && Math.floor(number)===number && number>=0 && number<=max?number:fallback;
}
function toggle(value,fallback){return value===true?1:value===false?0:integer(value,fallback,1);}
function normalize(raw){
 raw=raw||{};
 return {Palette:integer(raw.Palette,0,6),Background:integer(raw.Background,0,0xFFFFFF),
 TextColor:integer(raw.TextColor,0xFFFFFF,0xFFFFFF),HourColor:integer(raw.HourColor,0x55FFFF,0xFFFFFF),MinuteColor:integer(raw.MinuteColor,0xFFAA55,0xFFFFFF),
 TimeFormat:integer(raw.TimeFormat,0,2),DateFormat:integer(raw.DateFormat,0,3),Labels:toggle(raw.Labels,1),DigitalTime:toggle(raw.DigitalTime,1)};
}
var clay=new Clay([
 {type:'heading',defaultValue:'Vernier'},
 {type:'text',defaultValue:'Rotating hour and minute scales read beneath fixed top indices. The auxiliary seconds scale has ten intervals spanning nine minute intervals; each division represents six seconds.'},
 {type:'toggle',messageKey:'Labels',label:'Show seconds alignment scale',defaultValue:true},
 {type:'toggle',messageKey:'DigitalTime',label:'Digital time readout',defaultValue:true},
 {type:'select',messageKey:'TimeFormat',label:'Time format',defaultValue:'0',options:[
  {label:'Watch default',value:'0'},{label:'12-hour',value:'1'},{label:'24-hour',value:'2'}]},
 {type:'select',messageKey:'DateFormat',label:'Date at the top',defaultValue:'0',options:[
  {label:'Off',value:'0'},{label:'YYYY-MM-DD',value:'1'},{label:'MM/DD/YYYY',value:'2'},{label:'DD/MM/YYYY',value:'3'}]},
 {type:'heading',defaultValue:'Colors'},
 {type:'select',messageKey:'Palette',label:'Palette',defaultValue:'0',options:[
  {label:'Night',value:'0'},{label:'Paper',value:'1'},{label:'Mint',value:'2'},
  {label:'Ice',value:'3'},{label:'Rose',value:'4'},{label:'Monochrome',value:'5'},{label:'Custom colors',value:'6'}]},
 {type:'text',defaultValue:'Choose Custom colors to use the four colors below.'},
 {type:'color',messageKey:'Background',label:'Background',defaultValue:'000000',sunlight:false,allowGray:true},
 {type:'color',messageKey:'TextColor',label:'Numbers and text',defaultValue:'FFFFFF',sunlight:false,allowGray:true},
 {type:'color',messageKey:'HourColor',label:'Hour scale and alignment',defaultValue:'55FFFF',sunlight:false,allowGray:true},
 {type:'color',messageKey:'MinuteColor',label:'Minute scale',defaultValue:'FFAA55',sunlight:false,allowGray:true},
 {type:'submit',defaultValue:'Save to watch'}
],null,{autoHandleEvents:false});
function sendSaved(){
 var raw=localStorage.getItem('clay-settings');if(!raw)return;
 try{Pebble.sendAppMessage(normalize(JSON.parse(raw)));}catch(error){console.log('Vernier settings could not be loaded');}
}
Pebble.addEventListener('ready',sendSaved);
Pebble.addEventListener('showConfiguration',function(){Pebble.openURL(clay.generateUrl());});
Pebble.addEventListener('webviewclosed',function(event){
 if(!event || !event.response || event.response==='CANCELLED')return;
 try{clay.getSettings(event.response,false);sendSaved();}catch(error){console.log('Vernier settings were not saved');}
});
module.exports={normalize:normalize,defaults:defaults};
