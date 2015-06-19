#!/bin/sh
bin/type songss_rtags scan /media/seagate500GB/music/collection/flacs.cds /media/seagate500GB/music/collection/mp3s.only save
bin/type songss_rtags load test
bin/type songss_rtags load dump > /tmp/s_dump.txt
diff txt/albums.txt  txt/s_albums.txt  
diff txt/artists.txt  txt/s_artists.txt  
diff txt/genres.txt  txt/s_genres.txt  
diff txt/titles.txt txt/s_titles.txt
