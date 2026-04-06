#!/bin/bash
set -e

export LANG=en_US.UTF-8

#Install zip
if ! command -v 7z >/dev/null 2>&1; then
    echo "7z not found, installing..."
    apt update && apt install -y p7zip-full
fi

#FLASHCART
mkdir -p flashcart
cp -r Autoboot flashcart
cp -r _nds flashcart
cp -r _pico flashcart
cp -r ../language flashcart/_nds/akmenunext/language
cp ../akmenu-next.nds flashcart/boot.nds
cp ../akmenu-next.nds flashcart/_nds/akmenunext/launcher.nds
cp ../blocksds-bootloader/blocksds-bootloader.bin flashcart/_nds/akmenunext/load.bin
cd flashcart
7z a ../akmenu-next-flashcart.zip *
cd ..

#PICO
mkdir -p pico
cp -r _pico pico
cp -r _nds pico
cp -r ../language pico/_nds/akmenunext/language
cp ../akmenu-next_pico.nds pico/boot.nds
cp ../akmenu-next_pico.nds pico/_picoboot.nds
cp ../akmenu-next_pico.nds pico/_nds/akmenunext/launcher.nds
cp ../blocksds-bootloader/blocksds-bootloader.bin pico/_nds/akmenunext/load.bin
cd pico
7z a ../akmenu-next-pico.zip *
cd ..

#DSI
mkdir -p dsi
cp -r _nds dsi
cp -r title dsi
cp -r ../language dsi/_nds/akmenunext/language
cp ../akmenu-next.dsi dsi/boot.nds
cp ../akmenu-next.dsi dsi/akmenu-next.dsi
cp ../akmenu-next.dsi dsi/_nds/akmenunext/launcher.nds
cp ../blocksds-bootloader/blocksds-bootloader.bin dsi/_nds/akmenunext/load.bin
rm -f dsi/_nds/akmenunext/PassMeLoader.nds
cd dsi
7z a ../akmenu-next-dsi.zip *
cd ..

#3DS
mkdir -p 3ds
cp -r _nds 3ds
cp -r ../language 3ds/_nds/akmenunext/language
cp ../akmenu-next.dsi 3ds/boot.nds
cp ../akmenu-next.cia 3ds/akmenu-next.cia
cp ../akmenu-next.dsi 3ds/_nds/akmenunext/launcher.nds
cp ../blocksds-bootloader/blocksds-bootloader.bin 3ds/_nds/akmenunext/load.bin
rm -f 3ds/_nds/akmenunext/PassMeLoader.nds
cd 3ds
7z a ../akmenu-next-3ds.zip *
cd ..

#Cleanup
rm -rf flashcart dsi 3ds pico
