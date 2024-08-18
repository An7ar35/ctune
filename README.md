![cTune logo](docs/icon.svg "cTune logo")

cTune is a ncurses based internet radio player written in C for Linux.

Aside from playing a radio stream you can search and browse stations as well as keep a list of favourites.

It uses the RadioBrowser API to search and get radio stream information.


## Showcase video

[*Click here to watch the video (~45MB)*](https://www.eadavison.com/source/2023/ctune/Release%201.2/video/showcase.mp4)

## Features

- Play/search/browse radio stations using the RadioBrowser API
- Favourite stations from search/browse results
- Add/Edit custom local-only stations to favourites
- Sort favourites based on name/country/bitrate/codec/source/etc...
- Context-based help for key bindings (F1 key).
- Plugin system for playback (demuxing/resampling) and sound output (PCM)
- UI Themes
- Mouse support

### CLI

    Usage: ./ctune [OPTION]...

        --debug          prints out all debug messages to the log
    -f  --favourite      add station to favourites when used in conjunction with "--play"
    -h  --help           display this help and exits
    -p  --play "UUID"    plays the radio stream matching the RadioBrowser UUID
    -r  --resume         resumes station playback of the last session
        --show-cursor    always visible cursor
    -v  --version        prints version information and exits

## Application files

| type          | file name     | path                    | description                                                                  |
|---------------|---------------|-------------------------|------------------------------------------------------------------------------|
| executable    | `ctune`       | `/usr/bin/`             | cTune application binary                                                     |
| man           | `ctune.1.gz`  | `/usr/share/man/man1/`  | cTune man page                                                               |
| configuration | `ctune.cfg`   | `~/.config/ctune/`      | where the configuration is stored                                            |
| configuration | `ctune.fav`   | `~/.config/ctune/`      | where the favourite stations are stored                                      |
| logging       | `ctune.log`   | `~/.local/share/ctune/` | log file for last runtime (date/timestamps inside are UTC)                   |
| logging       | `playlog.txt` | `~/.local/share/ctune/` | playback log containing the stations and songs streamed during last runtime* |

*In case you want to find the name of a song/station that you liked and forgot to write down or favourite.

Both the application log and playback log are overwritten when `ctune` is launched again.

## Configuration

Most configuration options are accessible via the options menu.

Click [here](docs/guide/ctune.cfg.md) for a full breakdown of the configuration file's key-value pairs.

## Dependencies

| functionality | libraries                              |
|---------------|----------------------------------------|
| Network       | OpenSSL, POSIX sockets, Curl           |
| Playback      | FFMpeg/VLC, SDL2/PulseAudio/ALSA/sndio |
| Recording     | lame                                   |
| Parsing       | json-c (static)                        |


## Installation

### Compile from source

Requires the following to be installed on the system first:

- `cmake` version **3.17** and the CMake extra-modules package
- `git` for fetching the repos
- `ffmpeg`, `vlc` player libraries*
- `lame` for recording streams to mp3
- `curl` and `openssl` libraries
- `sdl2`  or `pulsedaudio` or `alsa` or `sndio` sound library/servers*
- `pandoc` and `gzip` for the man page

(*) The relevant plugins will be compiled for whatever libraries can be found on the system.

From there:

1. Clone the repository `git clone https://github.com/An7ar35/ctune.git`
2. Get in the directory with `cd ctune`
3. run `cmake . -DCMAKE_BUILD_TYPE=Release` (append ` -DCMAKE_INSTALL_PREFIX=</path/to/directory>` if you want to specify a custom directory for the installation)
4. run `cmake --build .`
5. run `sudo cmake --install .`
6. Done.

#### To uninstall

Just run `sudo xargs rm < install_manifest.txt` from within the cloned directory.

Or, alternatively:

- `cmake . -DCMAKE_BUILD_TYPE=uninstall`
- `sudo cmake --build . --target uninstall`

Finally, for both approaches, run `sudo mandb` after to purge the `ctune` entry for the man database

### Install from repository

#### Arch AUR (x64)

The package is available in the AUR repository under `ctune-git`. Install using your favourite AUR package browser/installer.

>  Alternatively just download the `PKGBUILD` file into an empty staging folder and run `makepkg -si` from inside. 
   The rest should take care of itself.

#### Ubuntu 22.10 'Kinetic' (tested with _pulseaudio_ as the default server)

<span style="color: orange;">The version of ffmpeg libs packaged with ubuntu 22.04.1 LTS are too old. Since the API changed I've updated the calls in ctune which breaks compilation for older ffmpeg libs.</span>

No PPA but here are copy/paste commands to install all the required programs and development libraries you would need before compiling `ctune`:

```shell
sudo apt-get install gcc libncurses5 git cmake cmake-extras make man pandoc gzip
```

```shell
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev libavdevice-dev libavfilter-dev libssl-dev libcurl4-openssl-dev libncurses5-dev libbsd-dev libpulse-dev lame
```

Once all these are on the system compiling from source should work without hiccups.

## Docker

Docker is only there for testing purposes and only works on linux with either `pulseaudio` or `pipewire-pulse` installed and running.
It uses the Arch docker image as base. The `docker-compose.sh` script creates the container and runs it (final size = ~1GB).

## Platform

Linux x64 with a UTF-8 locale.

As a baseline v1.1.5 works on ArchLinux with:

- FFMpeg (libavformat 59.27.100, libavcodec 59.37.100, libswresample 4.7.100)
- VLC (3.0.15)
- PulseAudio (14.2.0)
- SDL (2.0.14)
- ALSA (1.2.5)
- sndio (1.7.0)
- OpenSSL (1.1.1k)
- Curl (7.77.0)
- nCurses (6.4.20221231)
- libbsd
- lame (3.100)

## F.A.Q.

Q. **What are the key bindings?**

A. Press `F1` to get a contextual list of key bindings in the UI.

Q. **How do I use the mouse?**

A. Check out this [guide](docs/guide/mouse_navigation.md).

Q. **I'm getting weird symbols where the icons are supposed to be. What's going on?**

A. Your terminal font does not support the unicode characters used. Either change the font or switch back to ASCII icons from the Options menu.

Q. **Can I change the look?**

A. Yes. There are internal preset themes available in the Options menu and, if these don't strike your fancy, a custom theme can be specified inside ctune's configuration file.

## Bug reporting & Support

**Disclaimer: I've writen this software primarily for myself so temper your support-level expectations accordingly.**

That being said, if you find a bug you are welcome to open a ticket.
I'll try to deal with it time allowing. Same for bug PRs.

For tickets, please include the following to help diagnose the source of the problem:

1. Basic information:
    1. **(UI bugs)** Terminal/Shell used if it's a UI bug (e.g.: Konsole 21.04 using BASH 5.1.8)
    2. Version of cTune used  and what sound output/libraries it was compiled/run against (run `ctune --version` to get a print out of all that info)
2. Bug description
    1. What seems to break and where
    2. What triggers the bug - how did the bug manifest itself + steps to reproduce it
    3. **(UI bugs)** A screenshot of the issue
3. Logs and Configuration
    1. Configuration used (see inside `ctune.cfg`)
    2. The cTune error log (run `ctune --debug` to generate more granular and useful info during execution)
    3. Copy of the system log's (`syslog`) cTune runtime specific entries where the bug occurred (output of `journalctl --utc -b -0 | grep ctune` if you're using `systemd`)

Thank you.

## License

Copyright @ 2020-23 E.A.Davison.

Licensed under [AGPLv3](https://www.gnu.org/licenses/agpl-3.0.en.html)
