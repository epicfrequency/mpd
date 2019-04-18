
systemctl stop mpd
cp /usr/bin/mpd /usr/bin/mpdbackup
rm /usr/bin/mpd

cd /root
https://github.com/epicfrequency/MPD/blob/20.6-sacd-hifi/mpd
cp /root/mpd /usr/bin/mpdsacd
