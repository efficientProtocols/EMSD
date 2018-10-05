@echo off
rem RCS Version: $Id: invoke.bat,v 1.2 2002/10/25 19:37:44 mohsen Exp $

rem stress_i -l 14 -r 15 -p 2002 -d 2200 -t 20000 -n 192.168.0.10 -o stress_i.log 
rem stress_i -l 14 -r 15 -p 2002 -d 2200 -t 20000 -n 192.168.0.10 
stress_i -l 14 -r 15 -p 2002 -d 55 -t 20000 -n 192.168.0.10 -T DU_,ffff
