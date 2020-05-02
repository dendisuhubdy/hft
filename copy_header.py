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
  if not os.path.exists('./lib-hft'):
    print("%s not existed!"%('./lib-hft'))
    return
  dir_list = ['struct', 'util', 'core']
  header_path = 'lib-hft/include/'
  target_path = 'external/common/include/'
  for dl in dir_list:
    for f in os.listdir(header_path+dl):
      if f.split('.')[-1] == 'h' or f.split('.')[-1] == 'hpp':
        print('copying %s from %s to %s' % (f, header_path+dl, target_path+dl))
        command = 'cp -rf %s %s' % (header_path+dl+"/"+f, target_path+dl)
        os.system(command)
  for f in os.listdir(header_path):
    if f.split('.')[-1] != 'h' and f.split('.')[-1] != 'hpp':
      continue
    shutil.copy(header_path+f, target_path)
    print('copying %s to %s' % (header_path+f, target_path))
  shutil.copy("./lib-hft/lib/libnick.so", './external/common/lib')

if __name__ == '__main__':
  Copy()
