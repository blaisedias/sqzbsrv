#!/bin/sh
bin/s_rtags scan /media/seagate500GB/music/collection save
bin/s_rtags load test
bin/s_rtags load dump > /tmp/s_dump.txt
diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt
