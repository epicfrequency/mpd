
systemctl stop mpd
cp /usr/bin/mpd /usr/bin/mpdbackup
rm /usr/bin/mpd

cd /root
wget https://github.com/epicfrequency/MPD/blob/20.6-HiFi/bin/tinkerboard/mpd
cp /root/mpd /usr/bin/mpdsacd
