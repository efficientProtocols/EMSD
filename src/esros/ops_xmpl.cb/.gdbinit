# some useful commands for debugging emacs with gdb 4.* or better

echo Setting up the environment for debugging gdb.\n

#set args -l 12 -r 13 -p 2002 -n 192.168.0.10 -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff #-T USQ_,ffff
set args -l 15  -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff -T SCH_,01 -T 
#DU_,0400
show args

#file ./invoker
echo file ./invoker

set prompt (invoker-gdb) 

define performer
set args -l 13  -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff -T SCH_,01 -T 
#DU_,0400
show args
echo file ./performer
set prompt (performer-gdb) 
echo \n
end

document performer
Print Paramerter set for debugging the performer
end
