#!/bin/bash
cd "${BASH_SOURCE%/*}/" || exit
export PYU_SSH_USERNAME=furbrain
export PYU_SSH_KEYFILE=~/.ssh/id_rsa
export PYU_SSH_HOST=lynx.mythic-beasts.com
export PYU_SSH_REMOTE_PATH=/home/furbrain/www/www.shetlandattackpony.co.uk/releases/

mkdir src
for f in .pyupdater/config.pyu get_versions.py src/__init__.py src/client_config.py
do
  rm $f
  wget https://raw.githubusercontent.com/furbrain/PonyTrainer/master/$f -O $f
done

python3 get_versions.py
pyupdater keys -i keypack.pyu

for dir in new deploy files 
do
  rm pyu-data/${dir}/*
done

cp doc/manual.pdf firmware.hex pyu-data/new/
pyupdater archive --name manual.pdf --version $1
pyupdater archive --name firmware.hex --version $1

cp doc/manual.pdf firmware.hex pyu-data/new/
./pyupdater-win archive --name manual.pdf --version $1
./pyupdater-win archive --name firmware.hex --version $1

pyupdater pkg -p -s
./pyupdater-win pkg -p -s

pyupdater upload --service scp

