cd /root
wget https://github.com/epicfrequency/MPD/blob/20.6-HiFi/bin/tinkerboard/mpd

cp mpd /usr/bin/mpdsacd

wget  https://github.com/epicfrequency/MPD/blob/20.6-HiFi/volumio/UIConfig.json

cp /volumio/app/plugins/audio_interface/alsa_controller/UIConfig.json.bak

rm  /volumio/app/plugins/audio_interface/alsa_controller/UIConfig.json

cd /root

cp UIConfig.json  /volumio/app/plugins/audio_interface/alsa_controller/


