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

This fork is a work in progress to rewrite the UI part of **akmenu-next**. Currently it includes following changes:

- Rendering went through overhaul with bugfixes, new features, and code duplication removal.
- Font rendering was significantly enhanced, secondary small font added.
- Modal dialogs were rewritten from scratch to unblock main loop.
- Rom launchers were rewritten froms scratch with a brand new system of splitting tasks per frames.
- Many new UI elements were added like battery meter, boot icon, small date, small clock.
- Some UI elements enhanced, enriched, like settings tab with proper theme.
- First sound effects were created, like clock ticking sound.
- Improved visuals of auto booting last played games.
- New ways to display games.
- Many bug fixes.

The end goal is to recreate (with proper theme that's not included yet) the aesthetic and feel of original Nintendo DS bios.

Here are planned features:
- Analog clock.
- Tab icons in settings.
- Rewrite of input processing.
- Animated focus border in main list.
- Redesigned help menu.
- Button bar used in settings, modal dialogs, help menu.
- Top screen header used in settings, file manager, help menu.
- Navigation sounds.
- Animations.
- Redesigned start menu inspired by bios settings.

## Installing

Please check the [AKMenu-Next Docs](https://coderkei.github.io/akmenu-next-docs/) for setting up or updating AKMenu-Next.

## Building from source

Requires the pre-calico version of devkitarm to be installed with the `nds-dev` package.
A docker image is available with this version on [docker hub](https://hub.docker.com/layers/devkitpro/devkitarm/20241104).

Build the repository with `make`, then run `package.cmd/sh` depending on your OS to package the build for Flashcarts, DSi and 3DS.

### Local setup

Here is an example VS Code setup:
- Run `git submodule update --init` to install all submodules.
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
                "/opt/devkitpro/libnds/include/**"
            ],
            "defines": [
                "__arm__",
                "ARM9",
                "ARM11",
                "_GNU_SOURCE",
                "_NO_BOOTSTUB_"
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

* The system directory is `_nds/akmenunext` where the akmenu system files should be placed, along with a copy of the nds-bootstrap files in the `_nds` folder.
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
