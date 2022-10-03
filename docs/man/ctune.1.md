% CTUNE(1) ctune 1.0.13
% E.A.Davison
% June 2021

# NAME

ctune - C/NCurses internet radio tuner

# SYNOPSIS

**ctune** [*OPTIONS*]

# DESCRIPTION

**cTune** is a internet radio stream player. It can search/browse stations using the RadioBrowser.info API and keep a list of favourite stations from either search results or custom entries. 

# OPTIONS

**`--debug`**
:prints out all debug messages to the log

**`-f` | `--favourite`**
:add station to favourites when used in conjunction with "--play"

**`-h` | `--help`**
:display this help and exits

**`-p` | `--play "UUID"`**
:plays the radio stream matching the RadioBrowser UUID

**`-r` | `--resume`**
:resumes station playback of the last session

**`--show-cursor`**
:always visible cursor

**`-v` | `--version`**
:prints version information and exits

# EXIT VALUES

**0**
: Success

**1**
: Failure/Fatal error

# FURTHER HELP

Please refer to the in-application help dialog (default key: F1) for key bindings.

# COPYRIGHT

Copyright @ 2020-22 E.A.Davison. License AGPLv3 <https://www.gnu.org/licenses/agpl-3.0.en.html>.