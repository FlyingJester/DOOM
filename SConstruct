import os
import sys

environment = Environment();


environment.Append(CPPPATH=[os.path.join(os.getcwd(), 'extra')])

AddOption('--target', dest = 'target', default=os.name, help=\
"Set the target to build. Default is win32 on Windows and posix everywhere else.")

target = GetOption('target')

print target

if target=='posix':
    environment.Append(CPPDEFINES = ['NORMALUNIX', 'LINUX'])


SConscript(dirs=["doom"], exports = ["environment", "target"])
