#!/bin/sh

echo "creating html"
markdown manual.md > manual.html
echo "creating pdf"
markdown-pdf manual.md -o manual.pdf -p /usr/local/bin/phantomjs
echo "copying to server"
scp manual.html manual.pdf furbrain@lynx.mythic-beasts.com:www/www.shetlandattackpony.co.uk/static/

echo "updating opencart"
ssh furbrain@lynx.mythic-beasts.com /home/furbrain/update_sap_manual.py 
