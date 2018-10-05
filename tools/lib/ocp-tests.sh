
tm_setFile=ocp-tests.sh

 TM_trace 9 "hello" 
 LOG_event "hello" 
 LOG_message "Hello"
 EH_oops 
 EH_problem "Hello"
 #EH_fatal
 #EH_exit
