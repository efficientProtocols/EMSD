#
targetProd=baseua.mul
targetProdValidList="baseua.mul"

targetProdName=ETWP

targetReleaseNotesFile="${CURENVBASE}/relnotes/baseua-mul-beta.ttytex"

targetProdFlavor=consumer
targetProdFlavorValidList="consumer developer"

targetOS=wce
targetOSValidList="wce wce1.0 wce2.0"

targetArchType=sh3
targetArchTypeValidList="sh3 mips"

targetProdStatus=beta
targetProdStatusValidList="internal beta gold"

targetDistMethod=net-all
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


