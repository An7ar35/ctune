# CTune configuration file

The configuration is generated at first launch in `~/.config/ctune/ctune.cfg`. This is where some aspects of cTune can be customised.

### Editable values

| Configuration string                  | Value type   | Default value   | Description                                                                                                |
|---------------------------------------|--------------|-----------------|------------------------------------------------------------------------------------------------------------|
| `UI::Theme`                           | colour pair  | `{WHITE,BLACK}` | Base theme colours (foreground, background)                                                                |
| `UI::Theme::row`                      | colour pair  | `{WHITE,BLACK}` | Base theme colours for row entries (foreground, background)                                                |
| `UI::Theme::row::selected::focused`   | colour pair  | `{WHITE,BLUE}`  | Theme colours for selected and in-focus row entries                                                        |
| `UI::Theme::row::selected::unfocused` | colour pair  | `{BLACK,WHITE}` | Theme colours for selected and out-of-focus row entries                                                    |
| `UI::Theme::row::favourite::local`    | colour       | `MAGENTA`       | Text colour for a local based station's name on the row entry                                              |
| `UI::Theme::row::favourite::remote`   | colour       | `YELLOW`        | Text colour for a remote based station's name on the row entry                                             |
| `UI::Theme::icon::playback::on`       | colour       | `GREEN`         | Text colour for the playback icon when playing state is true ( <span style="color:green">></span> )        |
| `UI::Theme::icon::playback::off`      | colour       | `RED`           | Text colour for the playback icon when playing state is false ( <span style="color:red">.</span> )         |
| `UI::Theme::icon::queued`             | colour       | `CYAN`          | Text colour for the queued indicator on the corresponding row entry ( <span style="color:cyan">></span> )  |
| `UI::Theme::field::invalid`           | colour       | `RED`           | Text colour for an invalid field                                                                           |
| `UI::Theme::button`                   | colour pair  | `{WHITE,BLACK}` | Theme colours for buttons                                                                                  |
| `UI::Theme::button::invalid`          | colour       | `RED`           | Validation button colour when linked action fails                                                          |
| `UI::Theme::button::validated`        | colour       | `GREEN`         | Validation button colour when linked action is successful                                                  |

Colour values available: `BLACK`, `RED`, `GREEN`, `YELLOW`, `BLUE`, `MAGENTA`, `CYAN`, `WHITE`


### Auto-generated values and values set via the UI

It is not recommended to change these from the configuration file. Instead, use the 'options' > 'configuration' menu. 

| Configuration string             | Value type   | Default value  | Description                                                                                                                             |
|----------------------------------|--------------|----------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| `Resume::UUID`                   | UUID string  | AUTO-GENERATED | UUID of the last station playing during previous session                                                                                |
| `Resume::SourceID`               | unsigned int | AUTO-GENERATED | Source of the last station playing during previous session (local/radiobrowser/etc)                                                     |
| `Resume::Volume`                 | unsigned int | AUTO-GENERATED | Volume at last exit (`0`-`100`)                                                                                                         |
| `UI::Favourites::SortBy`         | unsigned int | AUTO-GENERATED | In-application selected default sorting for the entries in the Favourites tab                                                           |
| `IO::Plugin::Player`             | string       | `ffmpeg`       | Player plugin to use (`ffmpeg`, `vlc` )                                                                                                 |
| `IO::Plugin::SoundServer`        | string       | `pulse`        | Sound output plugin to use (`pulse`, `alsa`, `sdl`, `sndio`)                                                                            |
| `IO::Plugin::Recorder`           | string       | `mp3lame`      | Sound file output plugin to use (`mp3lame`, `wave`)                                                                                     |
| `IO::OverwritePlayLog`           | bool         | `true`         | Flag to overwrite play-log instead of appending to it                                                                                   |
| `IO::StreamTimeout`              | unsigned int | `5`            | Timeout value for streaming in seconds*                                                                                                 |
| `IO::NetworkTimeout`             | unsigned int | `8`            | Timeout value for the network calls in seconds                                                                                          |
| `IO::Recording::Path`            | string       | `""`           | Recording output directory                                                                                                              |
| `UI::Mouse`                      | bool         | `false`        | Flag to enable mouse support                                                                                                            |
| `UI::Mouse::IntervalPreset`      | integer      | `0` (default)  | Preset ID for the mouse click-interval resolution (time between a button press and a release for it to be registered as a click event)  |
| `UI::UnicodeIcons`               | bool         | `false`        | Flag to enable unicode icons (terminal font should support that)                                                                        |
| `UI::Favourites::ShowTheme`      | bool         | `true`         | Flag to show source theming on the Favourites tab                                                                                       |
| `UI::Favourites::UseCustomTheme` | bool         | `true`         | Flag to use the 'custom' preset's station source colouring instead of the currently selected preset's                                   |                                               |
| `UI::Favourites::UseLargeRows`   | bool         | `true`         | Flag to use large format row entries in the Favourites tab                                                                              |
| `UI::Search::UseLargeRows`       | bool         | `true`         | Flag to use large format row entries in the Search tab                                                                                  |
| `UI::Browser::UseLargeRows`      | bool         | `false`        | Flag to use large format row entries in the Browser tab                                                                                 |
| `UI::Theme::preset`              | string       | `default`      | Theme preset name                                                                                                                       |

Available theme preset values: "`default`", "`hackerman`", "`red-zone`", "`deep-blue`", "`custom`" (<- this will use the `UI::Theme::*` values in the configuration file)

(*) `StreamTimeout` fully works with the `ffmpeg` player currently and only stream testing with `vlc` (LibVLC doesn't have a call to set a custom value on the player).

**Note:** The VLC plugin is more of a fallback in the event that ffmpeg is an unviable option.