"""Build isolated deterministic native fixtures; never copy them into dist/."""
from pathlib import Path
import os,shutil,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 env={k:v for k,v in os.environ.items() if not k.startswith('VER_')};env.update(VER_FIXTURE='1',VER_HOUR='10',VER_MINUTE='10',VER_SECOND='20')
 output=ROOT/'.tmp/native';output.mkdir(parents=True,exist_ok=True)
 with (output/'fixture-build.log').open('w') as log:
  for command in [['pebble','clean'],['pebble','build']]:subprocess.run(command,cwd=ROOT/'watchface',env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
 for path,name in [('watchface.pbw','fixture.pbw'),('js/message_keys.json','message_keys.json')]:shutil.copy2(ROOT/'watchface/build'/path,output/name)
 print(output/'fixture.pbw')
if __name__=='__main__':main()
