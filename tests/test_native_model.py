"""Run the independent C geometry/placement oracle and actual Clay integration."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def test_complete_clock_pose_space():
 binary=ROOT/'.tmp/test_model_pytest'
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-O2','-I',str(ROOT/'watchface/src/c'),str(ROOT/'tests/host/test_model.c'),str(ROOT/'watchface/src/c/model.c'),'-lm','-o',str(binary)],check=True)
 subprocess.run([str(binary)],check=True)
def test_real_clay_save_cancel_reload():
 subprocess.run(['node','watchface/settings.test.js'],cwd=ROOT,check=True)
