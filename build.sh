cd /root

wget https://downloads.sourceforge.net/project/pupnp/pupnp/libUPnP%201.6.25/libupnp-1.6.25.tar.bz2
tar -xvf *.bz2
cd libupnp-1.6.25

./configure
make install

apt-get -y install build-essential automake autoconf libtool pkg-config libcurl4-openssl-dev intltool libxml2-dev libgtk2.0-dev libnotify-dev libglib2.0-dev libevent-dev dh-make  libboost-dev libmpdclient-dev libsystemd-dev libicu-dev libexpat1-dev libssl-dev libpugixml-dev libavformat-dev libflac-dev libcue-dev libsndfile-dev libid3tag0-dev libsoxr-dev

cd /root

git clone https://github.com/epicfrequency/MPD 

cd /root/MPD

./autogen.sh 
./configure --enable-sacdiso --disable-iso9660

make install -j6

cp /usr/local/bin/mpd /usr/bin/mpdsacd
systemctl stop mpd
rm /usr/bin/mpd
cp /usr/local/bin/mpd /usr/bin/

systemctl daemon-reload

systemctl start mpd




