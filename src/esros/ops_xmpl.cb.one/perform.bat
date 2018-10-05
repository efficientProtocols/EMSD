@echo off
rem #!/bin/sh
rem # RCS Version: $Id: perform.bat,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

rem performr -l 15  -T G_,ffff -T ESRO_,ffff  -T LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff -T DU_,ffff

performer.cb.one -l 15 -T UDP_,ffff -T DU_,ffff
