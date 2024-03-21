@ECHO OFF

ECHO Init everyone submodules...
git submodule update --recursive --init

ECHO Start checkout everyone submodules...
git submodule foreach --recursive git checkout master
git submodule foreach --recursive git pull
