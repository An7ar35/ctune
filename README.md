![cTune logo](docs/icon.svg "cTune logo")

cTune is a ncurses based internet radio player written in C for Linux.

Aside from playing a radio stream you can search and browse stations as well as keep a list of favourites.

It uses the RadioBrowser API to search and get radio stream information.

## Features

- Play/search/browse radio stations using the RadioBrowser API
- Favourite stations from search/browse results
- Add/Edit custom local-only stations to favourites
- Sort favourites based on name/country/bitrate/codec/source/etc...
- Context-based help for key bindings (F1 key).
- Plugin system for playback (demuxing/resampling) and sound output (PCM)

### CLI

    Usage: ./ctune [OPTION]...

        --debug          prints out all debug messages to the log
    -f  --favourite      add station to favourites when used in conjunction with "--play"
    -h  --help           display this help and exits
    -p  --play "UUID"    plays the radio stream matching the RadioBrowser UUID
    -r  --resume         resumes station playback of the last session
        --show-cursor    always visible cursor
    -v  --version        prints version information and exits

### nCurses

Click on picture to view a small "in-action" video (~6Mb).

[![video](docs/showcase/v1_0_0.png "Video preview")](docs/showcase/v1_0_0.mp4)


## Application files

| type | file name    | path | description |
| ---- | ------------ | ---- | ----------- |
| executable | `ctune` | `/usr/bin/` | cTune application binary |
| man | `ctune.1.gz` | `/usr/share/man/man1/` | cTune man page |
| configuration |`ctune.cfg` | `~/.config/ctune/` | where the configuration is stored |
| configuration |`ctune.fav` | `~/.config/ctune/` | where the favourite stations are stored |
| logging |`ctune.log` | `~/.local/share/ctune/` | log file for last runtime (date/timestamps inside are UTC) |
| logging |`playlog.txt` | `~/.local/share/ctune/` | playback log containing the stations and songs streamed during last runtime* |

*In case you want to find the name of a song/station that you liked and forgot to write down or favourite.

Both the application log and playback log are overwritten when `ctune` is launched again.

## Configuration

The configuration is generated at first launch in `~/.config/ctune/ctune.cfg`. This is where some aspects of cTune can be customised.

### Editable values

| Configuration string | Value type | Default value | Description |
| -------------------- | ---------- | ------------- | ----------- |
| `IO::Plugin::Player` | string | `ffmpeg` | Player plugin to use (`ffmpeg`, `vlc` ) |
| `IO::Plugin::SoundServer` | string | `pulse` | Sound output plugin to use (`pulse`, `alsa`, `sdl`, `sndio`) |
| `IO::OverwritePlayLog` | bool | `true` | Flag to overwrite play-log instead of appending to it |
| `IO::StreamTimeout` | unsigned int | `5` | Timeout value for streaming in seconds* |
| `IO::NetworkTimeout` | unsigned int | `8` | Timeout value for the network calls in seconds |
| `UI::Favourites::HideTheming` | bool | `false` | Flag to hide source theming on the Favourites tab |
| `UI::Favourites::UseLargeRows` | bool | `true` | Flag to use large format row entries in the Favourites tab |
| `UI::Search::UseLargeRows` | bool | `true` | Flag to use large format row entries in the Search tab |
| `UI::Browser::UseLargeRows` | bool | `false` | Flag to use large format row entries in the Browser tab |
| `UI::Theme` | colour pair | `{WHITE,BLACK}` | Base theme colours (foreground, background) |
| `UI::Theme::row` | colour pair | `{WHITE,BLACK}` | Base theme colours for row entries (foreground, background) |
| `UI::Theme::row::selected::focused` | colour pair | `{WHITE,BLUE}` | Theme colours for selected and in-focus row entries |
| `UI::Theme::row::selected::unfocused` | colour pair | `{BLACK,WHITE}` | Theme colours for selected and out-of-focus row entries |
| `UI::Theme::row::favourite::local` | colour | `MAGENTA` | Text colour for a local based station's name on the row entry |
| `UI::Theme::row::favourite::remote` | colour | `YELLOW` | Text colour for a remote based station's name on the row entry |
| `UI::Theme::icon::playback::on` | colour | `GREEN` | Text colour for the playback icon when playing state is true ( <span style="color:green">></span> ) |
| `UI::Theme::icon::playback::off` | colour | `RED` | Text colour for the playback icon when playing state is false ( <span style="color:red">.</span> ) |
| `UI::Theme::icon::queued` | colour | `CYAN` | Text colour for the queued indicator on the corresponding row entry ( <span style="color:cyan">></span> ) |
| `UI::Theme::field::invalid` | colour | `RED` | Text colour for an invalid field |
| `UI::Theme::button` | colour pair | `{WHITE,BLACK}` | Theme colours for buttons |
| `UI::Theme::button::invalid` | colour | `RED` | Validation button colour when linked action fails |
| `UI::Theme::button::validated` | colour | `GREEN` | Validation button colour when linked action is successful |

Colour values available: `BLACK`, `RED`, `GREEN`, `YELLOW`, `BLUE`, `MAGENTA`, `CYAN`, `WHITE`

(*) `StreamTimeout` fully works with the `ffmpeg` player currently and only stream testing with `vlc` (LibVLC doesn't have a call to set a custom value on the player).

**Note:** The VLC plugin is more of a fallback in the event that ffmpeg is an unviable option.

### Auto-generated values

| Configuration string | Value type | Description |
| -------------------- | ---------- | ----------- |
| `Resume::UUID` | UUID string | UUID of the last station playing during previous session |
| `Resume::SourceID` | unsigned int | Source of the last station playing during previous session (local/radiobrowser/etc) |
| `Resume::Volume` | unsigned int | Volume at last exit (`0`-`100`) |
| `UI::Favourites::SortBy` | unsigned int | In-application selected default sorting for the entries in the Favourites tab |

## Dependencies

| functionality | libraries |
|----------|-----------------------------------|
| Network  | OpenSSL, POSIX sockets, Curl      |
| Playback | FFMpeg/VLC, SDL2/PulseAudio/ALSA/sndio  |
| Parsing  | json-c (static)                   |


## Installation

### Compile from source

Requires the following to be installed on the system first:

- `cmake` version **3.17** and the CMake extra-modules package
- `git` for fetching the repos
- `ffmpeg`, `vlc` player libraries*
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

## Platform

Linux x64 with a UTF-8 locale.

As a baseline v1.0.0 works on ArchLinux with:

- FFMpeg (libavformat 58.76.100, libavcodec 58.134.100, libswresample 3.9.100)
- VLC (3.0.15)
- PulseAudio (14.2.0)
- SDL (2.0.14)
- ALSA (1.2.5)
- sndio (1.7.0)
- OpenSSL (1.1.1k)
- Curl (7.77.0)
- nCurses (6.2.20200212)

## F.A.Q.

Q. **What are the key bindings?**

A. Press `F1` to get a contextual list of key bindings in the UI.

## Bug reporting & Support

**Disclaimer: I've writen this software primarily for myself so temper your support-level expectations accordingly.**

That being said, if you open a bug ticket please include the following to help diagnose the source of the issue raised:

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

Copyright @ 2020-21 E.A.Davison.

Licensed under [AGPLv3](https://www.gnu.org/licenses/agpl-3.0.en.html)
