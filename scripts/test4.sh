#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
source $SCRIPT_DIR/medialocations
mkdir -p txt
mkdir -p data
rm txt/*.txt >& /dev/null
rm data/*.dat >& /dev/null

echo "=tracks_db=s_rtags=create=rtags=load=test=dump==="
echo ""
bin/s_rtags type tracks scan $media_locations save
bin/rtags load $media_locations test
bin/rtags load $media_locations dump > txt/dump.txt

echo ""
echo ""
echo ""
echo "=songs_db=s_rtags=create=load=test=dump========="
echo ""
bin/s_rtags type songs scan $media_locations save
bin/s_rtags type songs load $media_locations test
bin/s_rtags type songs load $media_locations dump > txt/s_dump.txt

diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt
#diff txt/dump.txt txt/s_dump.txt
fgrep -w TITLE txt/dump.txt | wc
fgrep -w ALBUM txt/dump.txt | uniq | wc
fgrep -w TITLE txt/s_dump.txt | wc
fgrep -w ALBUM txt/s_dump.txt | uniq | wc
