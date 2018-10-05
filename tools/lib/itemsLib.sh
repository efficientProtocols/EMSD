#!/bin/ksh

#
# RCS Revision: $Id: itemsLib.sh,v 1.7 2002/08/07 00:22:36 mohsen Exp $
#

#
# opItem_ library
# A set of facilities that operate on any item files
# 


# By adding 
#   iv_itemScopeVisibleHosts  -- List of hosts outside of the clusters, item is visible to
#   iv_itemScopeVisibleClusters   -- List of clusters, item is visible to
#   iv_itemScopeHiddenHosts -- List of hosts inside of the clusters, item is visible to
# you can then use
# opItem_isWithinClusterScope


# By adding 
#    iv_itemAvailableToHostModes
# you can then use
# opItem_isAvailableToHostMode


# By adding 
#   iv_itemAvailavleToOsType  -- matched against opRunOsType
#   iv_itemAvailavleToMachineArch  --  matched against opRunMachineArch
# you can then use
# opItem_isAvailableToOs



# BackWards Compatibility

function setBasicItemsFiles { opItem_selectClusterFiles $*; }

function itemIsAvailableToHostMode { opItem_isAvailableToHostMode $*; }

function itemIsWithinScope { opItem_isWithinClusterScope $*; }


function opItem_description {
  typeset tmp=`typeset +f | egrep '^iv_descriptionFunction'`
  if [[ "${tmp}" != "" ]] ;   then
    iv_descriptionFunction
  else
    print "No iv_descriptionFunction for ${iv_itemName}"
  fi
}

function opItem_selectClusterFiles {
    # $1 = base name for 

  fileBaseName=$1

  # Not yet, should this really be here
  opRunEnvGet
    
  thisItemsFiles=""
  
  if test -f ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.main ; then
    ItemsFiles="${ItemsFiles} ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.main"
    thisItemsFiles="SET"
  fi

  if test -f ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.${opRunClusterName} ; then
    ItemsFiles="${ItemsFiles} ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.${opRunClusterName}"
   thisItemsFiles="SET"
  fi

  if [ "${thisItemsFiles}X" == "X" ] ; then
    TM_trace 7 "Now Looking For ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.site"
    TM_trace 7 "opRunSiteName=${opRunSiteName} opRunClusterName=${opRunClusterName} opDomainName=${opDomainName}"
    if test -f ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.site ; then
      ItemsFiles="${ItemsFiles} ${opSiteControlBase}/${opRunSiteName}/${fileBaseName}.site"
    fi
  fi
}

function opItem_ifAvailableInvoke {
  # $* -- 
  # NOTYET, this is not complete

  typeset  retVal=0

  opItem_isAvailable || retVal=$?
  if [ "${retVal}" == 0 ] ; then
    $* || retVal=$?
  else
    TM_trace 5 "${iv_itemName} is not available to ${opRunHostName}."
  fi
  
  return ${retVal}

}


function opItem_isAvailable {
  # $* -- 
  # set -x
  typeset  retVal=0

  opItem_isAvailableToHostMode || retVal=$?
  if [ "${retVal}" == 0 ] ; then
      
    if [ "${opRunHostMode}_" = "clustered_" ] ; then

      retVal=0
      opItem_isWithinClusterScope || retVal=$?
      if [ "${retVal}" == 0 ] ; then
	retVal=0
      else
	TM_trace 5 "${iv_itemName} is not within cluster scope of ${opRunClusterName}."
      fi

    elif [ "${opRunHostMode}_" = "standalone_" -a "${opRunParamStandalone}_" == "clustered_" ] ; then 
      
      retVal=0
      opItem_isWithinClusterScope || retVal=$?
      if [ "${retVal}" == 0 ] ; then
	retVal=0
      else
	TM_trace 5 "${iv_itemName} is not within cluster scope of ${opRunClusterName}."
      fi

    else
      TM_trace 5 "${iv_itemName} is not available to ${opRunHostName}."
    fi
  fi
  
  return ${retVal}

}




function opItem_isAvailableToHostMode {
  # -- No args
  #
  # Returns: 
  #    0  if item is availabe to any of the specified host modes
  #    2  if item is not availabe to any of the specified host modes
  #

  if [ "X" == "X${iv_itemAvailableToHostModes}" ] ; then
    return 0
  fi
  
  typeset this=""


  for this in ${iv_itemAvailableToHostModes} ; do
    case ${this} in
      "clustered")
		   if [ "${this}_" == "${opRunHostMode}_" ] ; then
		     return 0
		   fi
		   ;;
		     
      "standalone")
		    if [ "${this}_" == "${opRunHostMode}_" -o  "${opRunHostMode}_" == "unnet_" ] ; then

		      # NOTYET, Backwards compatibility
		      #if [ "X${iv_itemStandaloneHostName}" == "X" ] ; then
		      #return 0
		      #fi

		      if [ "X${iv_itemStandaloneHostName}" == "Xall" ] ; then
			return 0
		      fi
		      
		      if [ "X${iv_itemStandaloneHostName}" != "X${opRunHostName}" ] ; then
			return 2
		      else
			return 0
		      fi
		    fi
		    ;;
		    
      "unnet")
	       if [ "${this}_" == "${opRunHostMode}_" ] ; then
		 return 0
	       fi
	       ;;

      "all")
	     return 0
	     ;;
      *)
	 EH_oops
	 return 2
	 ;;
    esac
  done
  return 2
}


function opItem_isAvailableToOs {
  # Requires no arguments
  #
  # Returns: 
  #    0  if item is availabe to any of the specified OS Types
  #    2  if item is not availabe to any of the specified host modes


  #set -x

  typeset this=""

  for this in ${iv_itemAvailavleToOsType} ; do
    if [ "X${this}" == "X${opRunOsType}" ] ; then
      return 0
    fi

    if [ "X${this}" == "Xall" ] ; then
      return 0
    fi
  done

  for this in ${iv_itemAvailavleToMachineArch} ; do
    if [ "X${this}" == "X${opRunMachineArch}" ] ; then
      return 0
    fi

    if [ "X${this}" == "Xall" ] ; then
      return 0
    fi
  done

  return 2
}





function opItem_isWithinClusterScope {
  # Requires no arguments
  #
  # Subject variables should be all set
  # Returns: 
  #    0  if disk within scope and should be acted upon
  #    1  if disk is tagged to be hidden
  #    2  if disk not in the cluster and also not tagged as visible
  #

  typeset thisVisHost=""
  typeset curVisCluster=""
  typeset thisHiddenHost=""

  for thisVisHost in ${iv_itemScopeVisibleHosts} ; do
    if [ "X${thisVisHost}" == "X${opRunHostName}" ] ; then
      return 0
    fi

    if [ "X${thisVisHost}" == "Xall" ] ; then
      return 0
    fi
  done


  for curVisCluster in ${iv_itemScopeVisibleClusters} ; do
    if [ "X${curVisCluster}" == "X${opRunClusterName}" -o "X${curVisCluster}" == "Xall" ] ; then
      TM_trace 7 "Yes in  ${opRunClusterName}"
      for thisHiddenHost in ${iv_itemScopeHiddenHosts} ; do
	if [ "X${thisHiddenHost}" == "X${opRunHostName}" ] ; then
	  return 1
	fi

	if [ "X${thisHiddenHost}" == "Xall" ] ; then
	  return 1
	fi

      done
      return 0
    fi
  done
  return 2
}

