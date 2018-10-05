# some useful commands for debugging emacs with gdb 4.* or better

echo Setting up the environment for debugging gdb.\n

set args -u 1000 -p 2002 -s /tmp/SPP -T LOPS_,ffff -T IMQ_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff -T DU_,0400 -o o$1.log -e e$1.log
#set args  -T DU_,0400 -T LOPS_,ffff -T IMQ_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff -T -p 1000 -o o1.log -e e1.log
#set args  -T LOPS_,ffff -T IMQ_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff -T SCH_,01 -p 1000 -o o$1.log -e e$1.log
#set args -o o$1.log -e e$1.log
#set args  -T EROP_,60 -p 1000
#set args -l -p 1000
show args

file ./esros 
echo file ./esros 

set prompt (prvdr-gdb) 

define example
output $.decl.name->identifier.pointer
echo \n
end

document example
Print the name of the decl-node that is $.
end
