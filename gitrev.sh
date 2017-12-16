#! /bin/bash
#
rev_new=$(git rev-parse --short=7 HEAD)
version=$(cat ./src/version.h 2>/dev/null | cut -d '"' -f2)


rev_date=`date -u +%Y%m%d%H%M%S`

    cat <<EOF > ./meta/meta.xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>SDcafiine</name>
  <coder>Chadderz Maschell</coder>
  <url>https://github.com/Maschell/SDCafiine</url>
  <version>$version-nightly-$rev_new</version>
  <release_date>$rev_date</release_date>
  <short_description>SDCafiine (HBL version)</short_description>
  <long_description>Put your files into "SD:/sdcafiine/TITLEID/MODPACK/content" or "SD:/sdcafiine/TITLEID//MODPACKaoc" where TITLEID is your 16 byte long TileID.

CREDITS : 
Cafiine creation - chadderz (and MrBean35000vr ?) 
Inital SDCafiine creation - golden45 (see https://gbatemp.net/goto/post?id=5680630) 
HBL support and further improvements - Maschell
minor improvements - Zarklord
Icon and Meta by pwsincd
  </long_description>
  <category>tool</category>

</app>
EOF

    cat <<EOF > ./src/version.h
#define APP_VERSION "$version-nightly-$rev_new"
EOF


echo $version-nightly-$rev_new
