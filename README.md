<p align="center">
   <img width="802" height="211" alt="image" src="https://github.com/user-attachments/assets/93b07874-5f3f-4572-b2e3-74bdabd757a0" />
</p>

<p align="center">
   <a href="https://gbatemp.net/threads/ds-i-3ds-akmenu-next-wood-frontend-for-nds-bootstrap.665743/">
      <img src="https://img.shields.io/badge/GBAtemp-Thread-blue.svg" alt="GBAtemp thread">
   </a>
   <a href="https://github.com/coderkei/akmenu-next/actions/workflows/main.yml">
      <img src="https://github.com/coderkei/akmenu-next/actions/workflows/main.yml/badge.svg" alt="Build status on GitHub Actions">
   </a>
</p>

## AKMenu-Next

Frontend for [nds-bootstrap](https://github.com/DS-Homebrew/nds-bootstrap/) and [Pico-Loader](https://github.com/LNH-team/pico-loader) based upon [lifehansolhacker's akmenu4 port](https://github.com/lifehackerhansol/akmenu4).

## Changes in this fork

- Small fixes to rendering and English typos.
- New setting to hide file extensions.
- Auto replace `;` from filenames with `:` when displaying text.
- Added a new file presentation mode `Focused`, which will hide system folders and files.
- Added a new file presentation mode `Minimal`, which will show all favorites and up to 20 games scanned from the sd card on the first page.
- Added a `textOffset` theme setting under `[main list]`.
- Added new theme ability to specify `main_list_item_bg.bmp` bg for main list items.
- Added new theme ability to specify `upper_screen_cal4.bmp` for upper screen bg when month has 4 rows.
- Added new theme ability to specify `upper_screen_cal5.bmp` for upper screen bg when month has 5 rows.
- Added new theme ability to specify `calendar/date_selection.bmp` for date selection bg.
- Added new theme ability to specify `calendar/day_numbers_second.bmp` for custom second digits in calendar.
- Added a `sundayColor` theme setting under `[calendar day]`.
- Added a `saturdayColor` theme setting under `[calendar day]`.
- Added a `fix ones` theme setting under `[calendar day]` to offset dates `21` and `31` on x axis by a specified value.

## Getting Started

Requires the pre-calico version of devkitarm to be installed with the `nds-dev` package.
A docker image is available with this version on [docker hub](https://hub.docker.com/layers/devkitpro/devkitarm/20241104).

Build the repository with `make`, then run `package.cmd/sh` depending on your OS to package the build for Flashcarts, DSi and 3DS.

### Local setup

Here is an example VS Code setup:
- In VS Code install extensions for `C++` and `Dev Containers`.
- Pull the docker imager from [docker hub](https://hub.docker.com/layers/devkitpro/devkitarm/20241104).
- Clone repo.
- Inside repo create folders `.devcontainer` and `.vscode`.
- Create file `.devcontainer/devcontainer.json` with content:
```json
{
    "image": "devkitpro/devkitarm:20241104", 
    "customizations": {
        "vscode": {
            "extensions": ["ms-vscode.cpptools"]
        }
    }
}
```
- Create file `.vscode/c_cpp_properties.json` with content:
```json
{
    "configurations": [
        {
            "name": "devkitARM-Internal",
            "includePath": [
                "${workspaceFolder}/arm9/build",
                "${workspaceFolder}/arm9/source",
                "${workspaceFolder}/arm9/source/font",
                "${workspaceFolder}/arm9/source/launcher",
                "${workspaceFolder}/arm9/source/saves",
                "${workspaceFolder}/arm9/source/ui",
                "${workspaceFolder}/share",
                "/opt/devkitpro/devkitARM/arm-none-eabi/include/**",
                "/opt/devkitpro/libgba/include/**",
                "/opt/devkitpro/libnds/include/**"
            ],
            "defines": [
                "__arm__",
                "ARM9",
                "ARM11",
                "_GNU_SOURCE"
            ],
            "compilerPath": "/opt/devkitpro/devkitARM/bin/arm-none-eabi-g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-arm"
        }
    ],
    "version": 4
}
```
- Open repo in VS Code, choose **Reopen in container** in the popup.
- In terminal run `make all` to generate all missing sources and headers.

At this point you should be ready to go.

### Configuration

* The system directory is `_nds/akmenunext` where the akmenu system files should be placed, along with a copy the nds-bootstrap files in the `_nds` folder.
* The binary of akmenu-next additionally needs to be in the `_nds/akmenunext` folder as `launcher.nds` for theme & language reboots, this is automatically added if using the package script.
* Cheats should be placed as `usrcheat.dat` into the `_nds/akmenunext/cheats` folder.
* Themes go into `_nds/akmenunext/ui`. Acekard & Wood R4 themes are supported.
* For flashcart related builds, files for the corresponding flashcart for Pico-Loader should go into the `_pico` folder.

## License

This project is comprised of various sources and is collectively licensed under the GPL-3.0-or-later license.
Please check [the license section](https://github.com/coderkei/akmenu-next/tree/main/licenses) for more information as well as a copy of all applicable licenses.

## Contributing

Contributions are welcome, any issues regarding game compatibility should submitted on the [nds-bootstrap](https://github.com/DS-Homebrew/nds-bootstrap) repository.

![image](https://www.gnu.org/graphics/gplv3-127x51.png)
