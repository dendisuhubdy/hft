import shutil
import subprocess
import sys
import os

def GetUserPath():
  user = os.getcwd().split('/')[1]
  if user == 'root':
    user_path = '/root/'
  elif user == 'home':
    user_path = '/home/' + os.getcwd().split('/')[2] + '/'
  else:
    print('unknown user!%s' %(os.getcwd()))
    sys.exit(1)
  return user_path


def Copy():
  if not os.path.exists(GetUserPath()+'lib-hft'):
    print("%s not existed!"%(GetUserPath()+'lib-hft'))
    return
  header_path = GetUserPath() + 'lib-hft/include/'
  target_path = GetUserPath() + 'hft/external/common/include'
  for root, dirs, files in os.walk(header_path):
    for f in files:
      src_file = os.path.join(root, f)
      if src_file.split('.')[-1] != 'h':
        continue
      shutil.copy(src_file, target_path)
      print('copying %s to %s' % (src_file, target_path))

if __name__ == '__main__':
  Copy()
