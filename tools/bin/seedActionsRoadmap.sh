NAME
     seedAction.sh

DESCRIPTION
     seedActions.sh is the basis of a tool for grouping
     a number of functions within a shell script and allowing
     for their execution and maintenance in a consistent way.

     A large number of common features are provided by simply
     loading seedActions.sh.  seedActions.sh integrates itself
     with your script in three stages.

     Below is the diagram of how this seedActions.sh works:

     seedActionsExample.sh      |       seedActions.sh
                                |
          1 |                   |
            |                   |
            +-------------------+--------------+ Configuration set:
                                               | - opConfig.sh
                                |              | Library load:
                                |            A | - ocp.lib
                                |              | GETOPT
            +-------------------+--------------+
          2 |                   |
 Default +  |                   |
 Mandatory  |                   |
 Parameters |                   |
            +-------------------+--------------+
                                |              | Set the user
                                |            B | define parameters
                POST            |              |
            +-------------------+--------------+
          3 |                   |
 vis_       |                   |
 functions  |                   |
            +-------------------+--------------+
                                |              | Execute
                                |            C | tasks
                                |              +



     In this description, the routine is:
        part 1 called --> part A executed -->
        part 2 called --> part B executed -->
        part 3 called --> part C executed.

     First, mmaExampleActions.sh is calling part 1:

        if [ "${loadFiles}X" == "X" ] ; then
            seedActions.sh -l $0 $@
            exit $?
        fi

     As a result, the seedActions.sh is executed and
     the first thing that seedActions.sh do is execute Part A:
        - load opConfig.sh
        - load ocp-lib.sh (OCP Library)
        - process GETOPT (get options)

     After Part A is executed, mmaExampleActions.sh declare the
     default parameter with tags (typeset -t) if any. 
     This is also known as PRE loading.

          typeset -t FirstName=MANDATORY
          typeset -t LastName=MANDATORY
          typeset -t SubsSelector=""
          .....

     This is where all of the necessary parameters are set,
     including the default and mandatory parameters.

     parameter=value from the command line must match a
     typeset -t.
     The initial value of mandatory variables is MANDATORY

     After all the parameters are set, seedActions.sh
     executes Part B:
        - set all of the user's define parameters.

     After we have all the parameters, part 3 is called
     (POST Loading).  Part 3 only executed if function
     called G_postParamHook exist within the script.

     command line "someFunction" maps to function: vis_someFunction

OPTIONS
     All scripts base on seedActions.sh get getopts with the
     following options:

     -T traceLevel      Use for debuging purposes -- tracing,
                        with traceLevel being a number
                        between 0-9.

     -i         Run a specific visible function within the
                script.

     -p         Specify the required/default parameters.
                parameter=value from the command line must match a
                typeset -t.  For example:
                   -p FirstName=Homer ...

     -l         Specify the file for loading.

     -u         Gives USAGE Info.  The usage info automatically
                lists all visible functions without the prefix "vis_".

VISIBLE FUNCTIONS
     - The visible functions (indicated by prefix vis_) are internal
       functions which are exposed externally.

     - It can accept ARGS on command line.

CONVENTIONS
     - In every script, vis_help is always put on top.
       The idea being that a description of the script
       can always be accessed through "-i help" in the
       command line.

     - Those based on seedActions.sh  should end in a category
       of actions as a VERB. The most generic form is the verb
       Action itself. For example:  mmaSendmailAction.sh

     - The noArgsHook function will be available in some
       of the script.
       If a default action is applicable to a script,
       the noArgsHook is called, if it exists,
       based on the recognition that a default action will be
       performed.
       If noArgsHook is not specified and the script is run with
       no options, then this warning will be displayed:
       "No action taken.  Specify options.  See -u"

EXAMPLE
     Mandatory parameters:
        the initial value of mandatory variables is MANDATORY
        e.g.
        typeset -t FirstName=MANDATORY
     
        In order to force this parameter to be set (hence MANDATORY)
        call the opParamMandatoryVerify within the function
        that needs this parameter.  When opParamMandatoryVerify is
        executed, it will check all of the parameters that has initial
        value MANDATORY.  If it is not set, return error.

     Optional parameters:
        the optional parameters has initial value other that MANDATORY.

     vis_help: the vis_help can always be accessed through "-i help"
               in the command line
     Example of usage: anyScript.sh -i help
     Example of code:

        vis_help () {
           cat  << _EOF_
             Put any text here for information related to this script.
           _EOF_

           exit 1
        }

     noArgsHook:
        e.g.

           noArgsHook="noArgsHook"
           noArgsHook() {
             # If no args, default action or usage

             if [ "$*X" == "X" ]
             then
               echo "No Defaults Specified"
               echo "Specify  Options -- See -u for list of visible actions"
               usage
             fi
           }



