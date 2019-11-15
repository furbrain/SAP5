#!/bin/sh
rm -fr gerbers
mkdir gerbers
cd gerbers
pcb -x gerber --gerberfile SAP --metric --name-style oshpark --all-layers ../SAP.pcb
pcb -x bom --xyfile SAP.xy --xy-unit mm ../SAP.pcb
pcb -x ps --multi-file --psfile SAP.ps ../SAP.pcb
../merge_files.py SAP.TXT SAP.XLN > SAP.DRL
cut -d "," -f 2 --complement SAP.xy | sed 's/# X/# X,Y in mm, rotation in degrees/' > SAP.pos
ps2pdf SAP.topassembly.ps SAP.assembly.pdf
zip SAP_gerbers.zip SAP.GB* SAP.GT* SAP.GKO SAP.DRL
echo "don't forget to edit the xy file to remove unused components!"
