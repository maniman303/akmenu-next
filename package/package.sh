#!/bin/bash
set -e

#Install zip
if ! command -v zip >/dev/null 2>&1; then
    echo "zip not found, installing..."
    apt update && apt install -y zip
fi

#FLASHCART
mkdir -p flashcart
cp -r Autoboot flashcart
cp -r _nds flashcart
cp -r _pico flashcart
cp ../akmenu-next.nds flashcart/boot.nds
cp ../akmenu-next.nds flashcart/_nds/akmenunext/launcher.nds
cd flashcart
zip -r ../akmenu-next-flashcart.zip *
cd ..

#PICO
mkdir -p pico
cp -r _pico pico
cp -r _nds pico
cp ../akmenu-next_pico.nds pico/boot.nds
cp ../akmenu-next_pico.nds pico/_picoboot.nds
cp ../akmenu-next_pico.nds pico/_nds/akmenunext/launcher.nds
cd pico
zip -r ../akmenu-next-pico.zip *
cd ..

#DSI
mkdir -p dsi
cp -r _nds dsi
cp -r title dsi
cp ../akmenu-next.dsi dsi/boot.nds
cp ../akmenu-next.dsi dsi/akmenu-next.dsi
cp ../akmenu-next.dsi dsi/_nds/akmenunext/launcher.nds
rm -f dsi/_nds/akmenunext/PassMeLoader.nds
cd dsi
zip -r ../akmenu-next-dsi.zip *
cd ..

#3DS
mkdir -p 3ds
cp -r _nds 3ds
cp ../akmenu-next.dsi 3ds/boot.nds
cp ../akmenu-next.cia 3ds/akmenu-next.cia
cp ../akmenu-next.dsi 3ds/_nds/akmenunext/launcher.nds
rm -f 3ds/_nds/akmenunext/PassMeLoader.nds
cd 3ds
zip -r ../akmenu-next-3ds.zip *
cd ..

#Cleanup
rm -rf flashcart dsi 3ds pico
