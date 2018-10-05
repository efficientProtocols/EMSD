#
# This File Name:  esros-srcFull-mul-lic-gold-local.sh
#
# Related files:  esros-srcFull-sol2-lic-gold-local.sh
#

# CVS tree that supports all product families
targetProd=esrosFull.mul
targetProdValidList="esrosFull.mul"


##
## Release Notes
#   targetReleaseNotesFile:  (historic)
#   targetReleaseNotesFamilyFile: Goes With the CVS Tree Used For ALL The builds.
#   targetReleaseNotesSpecificFile:  Goes with this specific build.
#

# ../relnotes/esrosFull-src-gold.ttytex 
targetReleaseNotesFile="${CURENVBASE}/relnotes/esrosFull-mul-gold.ttytex"
targetReleaseNotesFamilyFile="${CURENVBASE}/relnotes/esrosFull-mul-gold.ttytex"

targetReleaseNotesSpecificFile="${CURENVBASE}/relnotes/esros-srcFull-mul-lic-gold.ttytex"

targetBuildEnvFile="${CURENVBASE}/tools/buildEnv.sh"

targetProdName=ESROS

# NOTYET - Specification of the Target Header file also should go here

targetProdFlavor=srcFullLic
targetProdFlavorValidList="consumer developer srcFullLic srcOpen"

targetOS=sol2
targetOSValidList="sol2 nt4 wce wce1.0 wce2.0 default"

targetArchType=sun4
targetArchTypeValidList="sun4 sh3 mips"

targetProdStatus=gold
targetProdStatusValidList="internal beta gold"

targetDistMethod=local
targetDistMethodValidList="net-all disk local private"

targetDistFileBase=Neda-ETWP

targetDistUser=mohsen
#targetDistPasswd=

# These definitions should come last, 
# because they use the previous definitions
#
targetFtpInstallDirRelative="products/MobileMsgWare/${targetOS}/${targetDistFileBase}/${targetProdStatus}-${RELID_REVNUM}"
targetFtpInstallDir="/h8/var/ftp/pub/${targetFtpInstallDirRelative}"
targetWebInstallDirRelative="products/${targetDistFileBase}/${targetProdStatus}-${RELID_REVNUM}/${targetOS}"
targetWebInstallDir="/usr/public/doc/web/htdocs/${targetWebInstallDirRelative}"


