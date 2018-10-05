 #!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: win32SdkRoadmap.sh,v 1.1 2002/10/25 23:47:39 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi


function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- Information
${G_myName} -i overview
${G_myName} -i modelAndTerminology
${G_myName} -i todos
${G_myName} -i help
${G_myName} -i howTos
${G_myName} -i pointersAndReferences
${G_myName} -i printPackage
_EOF_
}


function noArgsHook {
  vis_examples
}

function vis_todos {
 cat  << _EOF_


Short Term:
----------


- Path to compiler should come from the user path.
  rest of the path is set in the scripts.
  Mohsen to take care of this.


Done
----

- 

_EOF_
}



function vis_overview {
 cat  << _EOF_
                                                   +=================+

_EOF_
}


function vis_modelAndTerminology {
 cat  << _EOF_

   Terminology and Model:
   ======================


_EOF_
}


function vis_help {
 cat  << _EOF_

At A Glance
-----------

  win32SdkRoadmap.sh       -- This File. General Orientation and Information

  win32sdk.libSh 

  win32cc.sh

  win32lib.sh

  win32link.sh

  win32rc.sh

_EOF_
}

function vis_howTos {
 cat  << _EOF_

 - How do I create a new Module mapFile?


_EOF_
}


function vis_pointersAndReferences {
 cat  << _EOF_


_EOF_
}

function vis_printPackage {
  a2ps -s 2 win32SdkRoadmap.sh  win32sdk.libSh win32.sh win32cc.sh win32lib.sh win32link.sh win32rc.sh 
}
