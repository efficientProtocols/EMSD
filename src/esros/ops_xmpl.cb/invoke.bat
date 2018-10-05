@echo off
rem #!/bin/sh
rem # RCS Version: $Id: invoke.bat,v 1.2 2002/10/25 19:37:36 mohsen Exp $

rem invoker -l 14 -r 15 -p 2002 -n 192.168.0.10 -T UDP_,ffff -T IMQ_,ffff -T EROP_,ffff -T ESRO_,ffff -T LOPS_,ffff -T SCH_,ffff 
rem invoker -l 14 -r 13 -p 2002 -n 192.168.0.25 -T UDP_,ffff -T IMQ_,ffff -T SCH_,ffff -T TMR_,ffff
rem  invoker -l 14 -r 15 -p 2002 -n 192.168.0.10 -T UDP_,ffff -T IMQ_,ffff -T TMR_,ffff
invoker.cb.one -l 14 -r 13 -p 2002 -n 192.168.0.25

