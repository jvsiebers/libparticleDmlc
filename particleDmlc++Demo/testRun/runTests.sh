#!/bin/bash
#
# 6/8/2015: JVS
# 9/11/2024: JVS: Convert to bash and test
#
#
# Setting home directory for the demo
demoHome="../.."
#
# create short name for the executable
particleDmlcDemo="$demoHome/particleDmlc++Demo/particleDmlc++Demo"
#
# the .so's must be in your path----I will temporarily extend LD_LIBRARY_PATH
# you should put the .so's wherever you keep other .so's
if [ -z "$UNAME" ]; then
    UNAME=$(echo "$(uname -p)-$(uname -s)" | tr '[:upper:]' '[:lower:]')
fi
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:$demoHome/$UNAME"
#
# the config files are expected to be in $UVA_DATA/dmlc, so set $UVA_DATA
export UVA_DATA="$demoHome/uva/data"
#
# Check the UVA_DATA/dmlc exists, report error if not
dmlc_dir=$UVA_DATA/dmlc
if [ -d "$dmlc_dir" ]; then
    echo "Directory $dmlc_dir exists."
else
    echo "Error: Directory $dmlc_dir does not exist."
    exit 1
fi

#
samplePhsp="inputPhaseSpace.egsphsp1"
#
#
# The file names tell which each type of dml file a file is
#
$particleDmlcDemo -dml fixedBeam.dml -ps $samplePhsp -z_plane 45
read -p $'\n Hit enter to continue to the next test> ' reply
#
$particleDmlcDemo -dml stepAndShoot.dml -ps $samplePhsp -z_plane 45
read -p $'\n Hit enter to continue to the next test> ' reply
#
$particleDmlcDemo -dml conformalArc.dml -ps $samplePhsp -z_plane 45 -oiaea -room
read -p $'\n Hit enter to continue to the next text> ' reply
#
$particleDmlcDemo -dml smartArc.dml -ps $samplePhsp -z_plane 45 -oiaea -room
#
echo -e "\n $0 completed\n"
