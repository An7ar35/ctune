### v1.3.2

- (Build) Added missing pipewire to auto-discovery

### v1.3.1

- (Bug) correction of the mouse enable/disable flag not propagating properly to all the widgets 
- (Bug) correction of the `Flag` enum string names (displayed in logs)

### v1.3.0

- (UI) Added mouse click interval resolution option
- (Plugin) FileOut plugin category created for recording currently played streams
- (plugin/output) new `wave` file output plugin added
- (plugin/output) new `mp3lame` file output plugin added
- (plugin/output) new `pipewire` audio output plugin added
- (plugin/input) added the stream timeout option from the config to `vlc`'s plugin
- (Bug) crash on resize resolved with event change queue between the backend and UI callbacks
- (Bug) pulse audio output broken on `pipewire-pulse` resolved by using a 'pull' paradigm and a `CircularBufffer` on the output plugins
- (Bug) added a missing volume function to sndio output plugin API
- (Bug/Network) changed network call to use the `curllib` instead and check the returned HTTP code, retry the query on another resolved server on the list and fail gracefully when these are exhausted
- (Bug) corrected RadioBrowser listing crash when an empty entry is in the list item by adding a check and fallback value in the json parser
- (Build) `CMakeList.txt` structure overhaul breaking down the plugins and the logger into their own sub-projects
- (Code) refactoring/cleaning up/minor bugs

