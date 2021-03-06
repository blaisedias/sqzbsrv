#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
source $SCRIPT_DIR/medialocations
mkdir -p txt
mkdir -p data
rm txt/*.txt >& /dev/null
rm data/*.dat >& /dev/null

echo "=tracks_db=rtags=create=rtags=load=test=dump==="
echo ""
time bin/rtags type tracks scan $mp3_locations save
time bin/rtags load test
time bin/rtags load dump > txt/dump.txt

echo ""
echo ""
echo ""
echo "=songs_db=s_rtags=create=load=test=dump========="
echo ""
time bin/s_rtags type songs scan $mp3_locations save
time bin/s_rtags type songs load test
time bin/s_rtags type songs load dump > txt/s_dump.txt

diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt
#diff txt/dump.txt txt/s_dump.txt
