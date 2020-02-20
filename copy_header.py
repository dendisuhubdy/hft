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
  dir_list = ['struct', 'util', 'core']
  header_path = GetUserPath() + 'lib-hft/include/'
  target_path = GetUserPath() + 'hft/external/common/include/'
  for dl in dir_list:
    print('copying dir %s to %s' % (header_path+dl, target_path+dl))
    command = 'cp -rf %s %s' % (header_path+dl, target_path)
    os.system(command)
  for f in os.listdir(header_path):
    if f.split('.')[-1] != 'h':
      continue
    shutil.copy(header_path+f, target_path)
    print('copying %s to %s' % (header_path+f, target_path))
  shutil.copy("/root/lib-hft/lib/libnick.so", '/root/hft/external/common/lib')

if __name__ == '__main__':
  Copy()
