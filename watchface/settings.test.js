var assert=require('assert'),Module=require('module'),path=require('path');
var original=Module._load,keys={};
require('./package.json').pebble.messageKeys.forEach(function(key,i){keys[key]=10000+i;});
var actual=path.join(__dirname,'node_modules/@rebble/clay/src/js/index.js'),config;
Module._load=function(name){
 if(name==='message_keys')return keys;
 if(name==='@rebble/clay'){var Clay=original.call(this,actual);return function(items,custom,opts){config=items;return new Clay(items,custom,opts);};}
 return original.apply(this,arguments);
};
var events={},storage={},sent=[];
global.localStorage={getItem:function(k){return storage[k]||null;},setItem:function(k,v){storage[k]=v;}};
global.Pebble={addEventListener:function(k,fn){(events[k]||(events[k]=[])).push(fn);},getAccountToken:function(){return 'test';},getWatchToken:function(){return 'test';},getActiveWatchInfo:function(){return {platform:'gabbro',model:'gabbro',language:'en_US',firmware:{major:4,minor:33,patch:1}};},sendAppMessage:function(v,ok){sent.push(v);if(ok)ok();},openURL:function(url){assert(url.indexOf('data:text/html')===0);}};
var settings=require('./src/pkjs/index');
function fire(name,event){(events[name]||[]).forEach(function(fn){fn(event);});}
assert.deepStrictEqual(settings.normalize({}),settings.defaults);
for(var p=0;p<=6;p++)assert.strictEqual(settings.normalize({Palette:String(p)}).Palette,p);
[null,'',false,-1,7,'oops',1.5].forEach(function(v){assert.strictEqual(settings.normalize({Palette:v}).Palette,0);});
for(var mode=0;mode<3;mode++)assert.strictEqual(settings.normalize({TimeFormat:String(mode)}).TimeFormat,mode);
for(var mode=0;mode<4;mode++)assert.strictEqual(settings.normalize({DateFormat:String(mode)}).DateFormat,mode);
assert.strictEqual(settings.normalize({Labels:false}).Labels,0);assert.strictEqual(settings.normalize({DigitalTime:true}).DigitalTime,1);
assert.strictEqual(settings.normalize({TextColor:0}).TextColor,0);
assert.strictEqual(settings.normalize({HourColor:0x1000000}).HourColor,0x55FFFF);
fire('ready');assert.strictEqual(sent.length,0);fire('showConfiguration');
['','CANCELLED','invalid'].forEach(function(response){fire('webviewclosed',{response:response});});assert.strictEqual(sent.length,0);
fire('webviewclosed',{response:encodeURIComponent(JSON.stringify({Palette:{value:'6'},Background:{value:0x000055},TextColor:{value:0xFFFFAA},HourColor:{value:0x55FFFF},MinuteColor:{value:0xFF55AA},TimeFormat:{value:'2'},DateFormat:{value:'3'},Labels:{value:true},DigitalTime:{value:false}}))});
assert.deepStrictEqual(sent[0],{Palette:6,Background:0x000055,TextColor:0xFFFFAA,HourColor:0x55FFFF,MinuteColor:0xFF55AA,TimeFormat:2,DateFormat:3,Labels:1,DigitalTime:0});
var saved=storage['clay-settings'];fire('webviewclosed',{response:'CANCELLED'});assert.strictEqual(storage['clay-settings'],saved);fire('ready');assert.deepStrictEqual(sent[1],sent[0]);
Object.keys(keys).forEach(function(key){assert(config.some(function(item){return item.messageKey===key;}));});
require('fs').writeFileSync(path.join(__dirname,'../.tmp/clay-save.json'),JSON.stringify(sent[0]));
console.log('PASS: real Clay Save/Cancel/reload, six presets/custom colors,12/24h, alignment scale, footer, dates');
