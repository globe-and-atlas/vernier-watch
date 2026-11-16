"""Run host checks and clean-build a production Vernier PBW."""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--host-only',action='store_true');args=parser.parse_args()
 temp=ROOT/'.tmp';temp.mkdir(exist_ok=True)
 binary=temp/'test_model'
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-O2','-I',str(ROOT/'watchface/src/c'),str(ROOT/'tests/host/test_model.c'),str(ROOT/'watchface/src/c/model.c'),'-lm','-o',str(binary)],check=True)
 subprocess.run([str(binary)],check=True)
 subprocess.run(['node','watchface/settings.test.js'],cwd=ROOT,check=True)
 if args.host_only:return
 if not shutil.which('pebble'):raise RuntimeError('Pebble CLI is required')
 env={k:v for k,v in os.environ.items() if not k.startswith('VER_')}
 for command in [['pebble','clean'],['pebble','build']]:subprocess.run(command,cwd=ROOT/'watchface',env=env,check=True)
 (ROOT/'dist').mkdir(exist_ok=True);target=ROOT/'dist/Vernier-0.1.0.pbw';shutil.copy2(ROOT/'watchface/build/watchface.pbw',target);print(target)
if __name__=='__main__':main()
