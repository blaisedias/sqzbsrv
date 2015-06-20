#!/bin/sh
rm txt/*.txt
rm data/*.dat
bin/s_rtags type tracks scan /media/seagate500GB/music/collection/flacs.cds /media/seagate500GB/music/collection/mp3s.only save
bin/rtags load test
bin/rtags load dump > /tmp/dump.txt

bin/s_rtags type songs scan /media/seagate500GB/music/collection/flacs.cds /media/seagate500GB/music/collection/mp3s.only save
bin/s_rtags type songs load test
bin/s_rtags type songs load dump > /tmp/s_dump.txt

diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt