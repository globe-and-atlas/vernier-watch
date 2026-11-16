"""Capture native motion/palettes with deterministic poses and complete LCD frames."""
from pathlib import Path
import argparse,json,shutil,time
from PIL import Image
from capture_native import ROOT,OUT,command,capture,monitors,close,cases
NAME='Vernier'

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--platform',choices=['emery','gabbro'],required=True);parser.add_argument('--reset-emulator',action='store_true');args=parser.parse_args();platform=args.platform
 store=ROOT/'media/store';store.mkdir(exist_ok=True)
 for port in monitors().values():close(port)
 command(['pebble','kill'])
 if args.reset_emulator:
  sdk=Path.home()/'Library/Application Support/Pebble SDK'
  for flash in sdk.glob('*/*/*flash*.bin'):
   backup=OUT/'emulator-backups'/flash.relative_to(sdk);backup.parent.mkdir(parents=True,exist_ok=True)
   if not backup.exists():shutil.copy2(flash,backup)
  command(['pebble','wipe'])
 install=['pebble','install','--emulator',platform,str(OUT/'fixture.pbw')];command(install);command(install)
 keys=json.loads((OUT/'message_keys.json').read_text());uuid=json.loads((ROOT/'watchface/package.json').read_text())['pebble']['uuid']
 def send(values):command(['pebble','send-app-message','--emulator',platform,'--app-uuid',uuid,'--int']+[f'{keys.get(k,k)}={v}' for k,v in values.items()])
 frames=[];durations=[]
 for palette in range(6):
  poses=[(10,10,20+i//4,i*250) for i in range(8)] if NAME=='Pendulum' else [(10,10,0,0),(10,20,12,0),(10,30,24,0),(10,40,36,0),(10,59,48,0),(11,0,0,0)]
  for hour,minute,second,phase in poses:
   values={**dict(cases()[0][1]),'Palette':palette,'Labels':0 if palette==5 else 1,11000:hour,11001:minute,11002:second,11022:1}
   if NAME=='Pendulum':values.update({'SmoothMotion':1,11003:phase})
   send(values)
   for attempt in range(30):
    im=capture();w=im.width
    if im.getpixel((w//2-3,20))==(255,0,255):assert im.getpixel((w//2+3,20))==(0,255,0);break
    time.sleep(.1)
   else:raise AssertionError('Fixture identity missing')
   send({11022:0});previous=None;stable=0
   for attempt in range(30):
    time.sleep(.12);im=capture();pixels=im.tobytes();stable=stable+1 if pixels==previous else 0;previous=pixels
    if stable>=2 and im.getpixel((w//2-3,20))!=(255,0,255):
     assert sum(px!=im.getpixel((0,0)) for px in im.crop((w//2-31,5,w//2+31,18)).get_flattened_data())>=10;break
   else:raise AssertionError('Native LCD frame did not settle')
   frames.append(im.copy());durations.append(250 if NAME=='Pendulum' else 600);print(platform,palette,hour,minute,second,phase,flush=True)
 target=store/f'{platform}_01_motion-colors.gif';frames[0].save(target,save_all=True,append_images=frames[1:],duration=durations,loop=0,disposal=2,optimize=False)
 with Image.open(target) as gif:assert gif.n_frames==(48 if NAME=='Pendulum' else 36) and gif.size==frames[0].size
 stills=['default','palette-1','half-hour','pure-analog'] if NAME=='Pendulum' else ['default','palette-1','late-12','midnight-24']
 for i,case in enumerate(stills,2):shutil.copy2(ROOT/f'media/{platform}-{case}.png',store/f'{platform}_{i:02d}_{case}.png')
 command(['pebble','kill'])
if __name__=='__main__':main()
