#
# This File Name:  %$Id:%
#
# 
#

# CVS tree that supports all product families
targetProd=OcpMul
targetProdValidList="OcpMul"


##
## Release Notes
#   targetReleaseNotesFile:  (historic)
#   targetReleaseNotesFamilyFile: Goes With the CVS Tree Used For ALL The builds.
#   targetReleaseNotesSpecificFile:  Goes with this specific build.
#

# ../relnotes/ocp-srcFullPub-mul-gold.ttytex 
targetReleaseNotesFile="${CURENVBASE}/relnotes/ocp-srcFullPub-mul-gold.ttytex"
targetReleaseNotesFamilyFile="${CURENVBASE}/relnotes/OcpMul-gold.ttytex"

targetReleaseNotesSpecificFile="${CURENVBASE}/relnotes/ocp-srcFullPub-mul-gold.ttytex"

targetBuildEnvFile="${CURENVBASE}/tools/buildEnv.sh"

targetProdName=OCP

# NOTYET - Specification of the Target Header file also should go here

targetProdFlavor=srcFullPub
targetProdFlavorValidList="consumer developer srcFullLic srcFullPub srcOpen"

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
