NAME
     seedSubjectAction.sh

DESCRIPTION
     seedSubjectAction.sh is the basis of a tool for grouping
     a number of functions within a shell script and allowing
     for their execution and maintenance in a consistent way.

     A large number of common features are provided by simply
     loading seedSubjectAction.sh.  seedSubjectAction.sh integrates itself
     with your script in three stages.

     Below is the diagram of how this seedSubjectAction.sh works:

     procSubjects.sh.sh |   seedSubjectAction.sh        |  procSubjectItems.main
                        |                               |
          1 |           |                               |
            |           |                               |       
            +-----------+--------------+ Conf. set:     |
			|	       | - opConfig.sh  |
                        |              | Library load:  |
                        |            A | - ocp.lib      |
                PRE     |              | GETOPT         |
            +-----------+--------------+                |
          2 |           |                               |
 Default +  |           |                               |
 Mandatory  |           |                               |       
 Parameters |           |                               |
            +-----------+--------------+                |
                        |              | Set the params |
                        |            B |                |
                POST    |              |                |
            +-----------+--------------+                |
          3 |           |                               |
 do_        |           |                               |
 item_      |           |                               |
 functions  |           |                               |
 itemFiles  +-----------+--------------+                |
 specified here         |              |                |
                        |              +----------------+-------+
                        |                               |       | procSubjectItems.
                        |              +----------------+-------+
                        |              |                |
                        |           C  +----------------+-------+
                        |                               |       | procSubjectItems. N
                        |              +----------------+-------+
                        |              |                |
                        |              |                |
                        |              +                |

     In this description, the routine is:
        part 1 called --> part A executed -->
        part 2 called --> part B executed -->
        part 3 called --> part C executed.

     First, seedSubjectActionExample.sh is calling part 1:

        if [ "${loadFiles}X" == "X" ] ; then
            seedSubjectAction.sh -l $0 $*
            exit $?
        fi

     As a result, the seedSubjectAction.sh is executed and
     the first thing that seedSubjectAction.sh do is execute Part A:
	- load opConfig.sh
        - load ocp-lib.sh (OCP Library)
        - process GETOPT (get options)

     After Part A is executed, seedSubjectActionExample.sh declare the
     default parameter with tags (typeset -t) if any. 
     This is also known as PRE loading.

        if [ "${loadSegment}_" == "PRE_" ] ; then
          # Mandatory parameters
          typeset -t VirDomRoot=MANDATORY
          typeset -t VirDomTLD=MANDATORY
          
          # Optional parameter = default value
          typeset -t SiteName=xyzPlus
          .....

     This is where all of the necessary parameters are set,
     including the optional and mandatory parameters.

     parameter=value from the command line must match a
     typeset -t.
     The initial value of mandatory variables is MANDATORY
     and the optional parameters become the default value.

     After all the parameters are set, seedSubjectAction.sh
     executes Part B:
        - set all of the user's define parameters.

     After we have all the parameters, part 3 is called
     (POST Loading). Part 3 only executed if function
     called G_postParamHook exist within the script.

     The setBasicItemsFile is called here. See CONVENTIONS
     section for how setBasicItemsFiles works.
     The itemsFile are loaded from the procSubjectItems file:
         procSubjectItems.<specificCluster>
     where procSubjetItems is the corresponding procSubjects.sh,
     <specificSite> is one of main, office, public, etc.

     When procSubjectItems is executed, itemPre and itemPost 
     are defined, if there is any.

     itemPre is a place where all the default and mandatory
     parameters are specified.

     itemPost derived defaults.

     After the itemsFile is loaded, "subject" and "action" 
     are defined.

     command line "subject" maps to function:  item_subject
     command line "action"  maps to function:  do_action
     By convention, it calls itemAction_action.


OPTIONS
     All scripts base on seedSubjectAction.sh get getopts with the
     following options:

     -T traceLevel      Use for debuging purposes -- tracing,
                        with traceLevel being a number
                        between 0-9.

     -a         Run the specific action.  The "action"
                automatically lists all the action available
                without the "do_" prefix.
                Also applies to itemCmd_ as well.

     -s         Apply the -a "action" to a specific "subject".  
                The "subject" automatically lists all the subject 
                available without the "item_" prefix.

     -i         Run a specific visible function within the
                script.

     -p         Specify the required/default parameters.
                parameter=value from the command line must match a
                typeset -t.  For example:
                   -p FirstName=Homer ...

     -l         Specify the file for loading.

     -u         Gives USAGE Info.  The usage info automatically
                lists all visible functions without the prefix "vis_".

CONVENTIONS
     - In every script, vis_help is always put on top.
       The idea being that a description of the script
       can always be accessed through "-i help" in the
       command line.

     - Those based on seedSubjectAction.sh should end
       in the plural of the OBJECT, if there are categories
       of actions related to the objects those as verbs come
       before the plural of the object.
       For example: opDiskDrives.sh or mmaQmailHosts.sh

       The seed of the items file is the singular of the fileName
       plus Items. For example opDiskDriveItems.sh or mmaQmailHostItems.sh.

     - The noArgsHook function will be available in some
       of the script.
       If a default action is applicable to a script,
       the noArgsHook is called, if it exists,
       based on the recognition that a default action will be
       performed.
       If noArgsHook is not specified and the script is run with
       no options, then this warning will be displayed:
       "No action taken.  Specify options.  See -u"

     - The noSubjectHook function will be available in some
       of the script.
       This function will be executed if there is no subject
       specified.

     - The firstSubjectHook and lastSubjectHook are typically
       used when the subject is all.  Most of the time, it will
       be used for printing summary of the itemsFile.

     - setBasicItemsFiles procSubjectItems
       Here are the flow how setBasicItemsFiles works:
        if there is procSubjectItems.main, then add it.
        if there is procSubjectItems.clusterName, then add it.
        
        if there is none of the above then
        if there is procSubjectItems.site, then add it.

        if there is procSubjectItems.otherName, just ignore it.

        Here is a scenario:
        - For example, suppose we have all of these files:
          procSubjectItems.main, procSubjectItems.office,
          procSubjectItems.home, procSubjectItems.otherCluster
          and we are running from an office machine environment
          then only procSubjectItems.main and procSubjectItems.office
          are loaded and the other are ignored.
        
     - The itemsFile policy:
        item_SSSS  (SSSS is the subject)
        itemPre
        iv_specialize
        itemPost
        itemCmd_ 

     - Built in function:
        list -- built in action
        all  -- built in subject
       Example of use in command line:
        anyScript.sh -s all -a list
       This command will enumerate all the subject item_ entries from 
       the ItemsFile and list all of the paramaters corresponding to
       each subject item_.

EXAMPLE
     Mandatory parameters: 
     ---------------------
        the initial value of mandatory variables is MANDATORY
        e.g.
        typeset -t FirstName=MANDATORY

     Optional parameters:
     --------------------
        typeset -t FirstName=homer

     vis_help: 
     ---------
     the vis_help can always be accessed through "-i help"
     in the command line
     Example of usage: anyScript.sh -i help
     Example of code:

        vis_help () {
           cat  << _EOF_ 
             Put any thext here for information related to this script.
           _EOF_

           exit 1
        }

     noArgsHook: 
     -----------
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

     Use of parameters in vis_ function:
     -----------------------------------
     print ${FirstName} will give result "homer".

     ItemsFile Selection:
     --------------------
     There are 2 ways to load the procSubjectItems:
        1. Automatic ItemsFile Selection
                
                setBasicItemsFiles procSubjectItems

        2. Manual ItemsFile Selection
                
                ItemsFile=${opSiteControlBase}/${opSiteName}/procSubjectItems.main

     do_ description:
     ----------------
     The do_AAA function is the AAA "action" taken to some 
     "subject" item_.
     By convention it calls itemAction_AAA.

     itemCmd_ description:
     ---------------------


