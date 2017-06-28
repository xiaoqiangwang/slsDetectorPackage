GITREPO1='git remote -v'
GITREPO2=" | grep \"fetch\" | cut -d' ' -f1"
BRANCH1='git branch -v'
BRANCH2=" | grep '*' | cut -d' ' -f2"
REPUID1='git log --pretty=format:"%H" -1'
AUTH1_1='git log --pretty=format:"%cn" -1'
AUTH1_2=" | cut -d' ' -f1"
AUTH2_1='git log --pretty=format:"%cn" -1'
AUTH2_2=" | cut -d' ' -f2"
FOLDERREV1='git log --oneline . '   #used for all the individual server folders
FOLDERREV2=" | wc -l"  #used for all the individual server folders
REV1='git log --oneline  '
REV2=" | wc -l"
RDATE1='git log --pretty=format:"%ci" -1'
COMMIT_TITLE_SCRIPT='git log --pretty=format:"%s" -1'

cd .. 
COMMIT_TITLE_SCRIPT='git log --pretty=format:"%s" -1 eigerDetectorServer'
COMMIT_TITLE=`eval $COMMIT_TITLE_SCRIPT`
echo $COMMIT_TITLE
if [ "$COMMIT_TITLE" == "updatereveiger" ]; then
echo "No update"
elif [ "$COMMIT_TITLE" == "updaterev" ]; then
echo "No update"
else
cd eigerDetectorServer
GITREPO=`eval $GITREPO1  $GITREPO2`
BRANCH=`eval $BRANCH1  $BRANCH2`
REPUID=`eval $REPUID1`
AUTH1=`eval $AUTH1_1  $AUTH1_2`
AUTH2=`eval $AUTH2_1  $AUTH2_2`
REV=`eval $REV1  $REV2`
FOLDERREV=`eval $FOLDERREV1  $FOLDERREV2`
RDATE=`eval $RDATE1`
echo Path: slsDetectorsPackage/slsDetectorSoftware/eigerDetectorServer  $'\n'URL: ${GITREPO}/eigerDetectorServer  $'\n'Repository Root: ${GITREPO}  $'\n'Repsitory UUID: ${REPUID}  $'\n'Revision: ${FOLDERREV}  $'\n'Branch: ${BRANCH}  $'\n'Last Changed Author: ${AUTH1}_${AUTH2}  $'\n'Last Changed Rev: ${REV}  $'\n'Last Changed Date: ${RDATE} > gitInfo.txt 
cd ../../
./genVersionHeader.sh slsDetectorSoftware/eigerDetectorServer/gitInfo.txt slsDetectorSoftware/eigerDetectorServer/gitInfoEigerTmp.h slsDetectorSoftware/eigerDetectorServer/gitInfoEiger.h 
echo "Revision Updated"
cd slsDetectorSoftware
fi

cd eigerDetectorServer
