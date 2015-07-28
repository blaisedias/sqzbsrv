#!/bin/sh
rm txt/*.txt
rm data/*.dat
time bin/s_rtags type tracks scan /media/seagate500GB/music/collection/flacs.cds /media/seagate500GB/music/collection/mp3s.only save
time bin/rtags load test
time bin/rtags load dump > txt/dump.txt

echo ""
echo "========================================"
echo ""
echo ""

time bin/s_rtags type songs scan /media/seagate500GB/music/collection/flacs.cds /media/seagate500GB/music/collection/mp3s.only save
time bin/s_rtags type songs load test
time bin/s_rtags type songs load dump > txt/s_dump.txt

diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt
#diff txt/dump.txt txt/s_dump.txt
