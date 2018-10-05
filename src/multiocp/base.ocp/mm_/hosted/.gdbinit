echo Setting up the environment for debugging gdb.\n

echo Which Program to run? (agent), (manager):

define agent
#set args -T MM,ffff -T ASN,ffff
set args -T MM,ffff
show args
file ./test_a
echo file ./test_a
set prompt (test_a-gdb) 
echo \n
end

document agent
Print Parameter set for debugging the test_a
end



define manager
#set args -T MM,ffff -T ASN,ffff localhost
set args -T MM,ffff localhost
show args
file ./test_m
echo file ./test_m
set prompt (test_m-gdb) 
echo \n
end

document manager
Print Parameter set for debugging the test_m
end
