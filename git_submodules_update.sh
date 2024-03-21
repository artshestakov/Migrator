#!/bin/bash

echo Init everyone submodules...
git submodule update --recursive --init

echo Start checkout everyone submodules...
git submodule foreach --recursive git checkout master
git submodule foreach --recursive git pull
