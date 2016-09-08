#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
source $SCRIPT_DIR/medialocations
mkdir -p txt
mkdir -p data
echo "=songs_db=s_rtags=create=load=test=dump========="
echo ""
bin/s_rtags type songs scan $media_locations save
bin/s_rtags type songs load $media_locations test
bin/s_rtags type songs load $media_locations dump > txt/s_dump.txt

diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt
