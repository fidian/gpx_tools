GPX Tools
=========

This collection of free tools will manipulate a GPX file that you get from [Geocaching.com](http://www.geocaching.com) when you are a premium member.  It will rewrite the XML inside the file before you load it into your GPS.  To get your first GPX file you will need to create a Pocket Query and get the resulting list of caches.

With these tools you can change the names, descriptions, and symbols for the waypoints in a very flexible manner.  It is similar to how [GPX Spinner](http://www.gpxspinner.com) and [GSAK](http://gsak.net/) will alter waypoints before sending them to your GPS receiver.  You are also able to filter out waypoints to have a certain sizes, types, difficulty, and terrain.  When coupled with [GPSBabel](http://www.gpsbabel.org/), you have a powerful system that can narrow waypoints geographically and send them to your GPS receiver
as well.


`gpxinfo`
---------

`gpxinfo` shows aggregate information about all of the geocaches listed in a GPX file.  Find out the number of waypoints, the different difficulty levels, the number of caches with a specific container size, breakdowns of attributes, and more.

### Syntax

    gpxinfo FILENAME

Example:

    gpxinfo my_pocket_query.gpx

It's pretty self-explanatory.  You can use "-" instead of a source file to read the GPX file from stdin.  In the end, you will get something like this:

    Waypoints:  439
    Available:  439
    Archived:  0
    ID Range:  662 - 95094
    Latitude Range:  43.500717 - 48.603901
    Longitude Range:  -96.623802 - -90.014969
    Difficulty Counts:
            1:  113
            1.5:  110
            2:  135
            2.5:  45
            3:  26
            3.5:  5
            4:  4
            4.5:  1
    Terrain Counts:
            1:  103
            1.5:  107
            2:  100
            2.5:  45
            3:  39
            3.5:  27
            4:  7
            4.5:  1
            5:  10
    Type Counts:
            Traditional Cache:  356
            Virtual Cache:  49
            Multi-cache:  19
            Unknown Cache:  13
            Letterbox Hybrid:  2
    Size Counts:
            Regular:  324
            Virtual:  44
            Micro:  22
            Small:  18
            Other:  16
            Not chosen:  12
            Large:  3

There really isn't much to say about this tool.  I wrote it to make sure that other tools were doing their job.


`gpxfilter`
-----------

[`gpxfilter`](docs/gpxfilter.md) removes waypoints from a GPX file that do not meet your specific requirements.  It can be used to filter out caches based on difficulty, terrain, size, or cache type.


### Syntax

    gpxfilter gpx_in.gpx gpx_out.gpx [filter [filter [...]]]

Example:

    gpxfilter my_query.gpx filtered_list.gpx -maxterr 2 -mindiff 3.5 -size MSRL -type TU

You can use "-" instead of gpx_in.gpx and gpx_out.gpx to use stdin of stdout instead of an actual file.

### Filters

* `-mindiff ###`
    * Sets the minimum difficulty level to preserve.
    * Example to keep all caches with at least a difficulty of 1.5:  `-mindiff 1.5`

* `-maxdiff ###`
    * Sets the maximum difficulty level that should be kept.
    * Example to eliminate caches with a 4.5 or 5 difficulty:  `-maxdiff 4`

* `-minterr ###`
    * Sets the minimum allowable terrain value for geocaches.
    * Example to keep terrains that are over a 2:  `-minterr 3.5`

* `-maxterr ###`
    * Removes all caches with a terrain rating greater than the value specified.
    * Example to eliminate terrains over a 4: `-maxterr 4`

* `-size ....`
    * Sets what sizes are allowed.  If you do not specify a size code here,  geocaches of that size will not make it through the filter.  If you do not use this setting at all, then it is the same as specifying all sizes.
    * Codes: U = Unknown, M = Micro, S = Small, R = regular, L = Large, V = Virtual, O = Other
    * Example to keep only physical caches with a listed size (micro through large):  `-size MSRL`

* `-type ....`
    * Keeps only geocaches with the listed container types.
    * Codes: X = Benchmark, C = Cache In Trash Out Event, G = Earthcache, E = Event Cache, B = Letterbox Hybrid, L = Locationless (Reverse) Cache, E = Mega-Event Cache, M = Multi-cache, A = Project APE Cache, T = Traditional Cache, U = Unknown Cache, V = Virtual Cache, W = Webcam Cache
    * Example to keep only traditionals, multi-caches, and webcams: `-size TMW`


`gpxrewrite`
============

Rewrite a GPX file so that you change the name, description, and symbol tags for the caches.  This can provide more information at your fingertips by giving geocaches custom symbols for different sizes and add extra information to the name and description of your geocaches.

### Syntax

    gpxrewrite settings_file [gpx_in.gpx [gpx_out.gpx]]

Example:

    gpxrewrite settings.ini my_query.pgx reformatted.gpx

The settings file is where all of the formats, symbols, and other settings are kept.  It is the only parameter that must be specified on the command line.

The input GPX file must be one from a Geocaching.com Pocket Query.  Unknown things can happen if you try to reformat a standard GPX file that is lacking the extra groundspeak attributes.  If you do not specify a file, you can instead pass one in with stdin.

The output GPX file is where the rewritten GPX file will be saved.  If not specified, the file is written to stdout (the screen).  This will overwrite an existing file without prompting.


### Settings File

`gpxrewrite` requires a settings file, which I have called `settings.ini` in my example.  Settings are specified in a key/value pair, one per line.  Keys are case insensitive, and the last one specified in the file is the one that is used.

* `Benchmark_Prefix`, `CITO_Event_Prefix`, `Earthcache_Prefix`, `Event_Prefix`, `Letterbox_Hybrid_Prefix`, `Locationless_Prefix`, `Mega_Prefix`, `Multi_Prefix`, `Project_APE_Prefix`, `Traditional_Prefix`, `Unknown_Prefix`, `Virtual_Prefix`, `Webcam_Prefix`

    * These codes specify what will replace `%C` format codes.  If you do not change them, a default one-leter code is assigned for each prefix.  See the sample settings file below.

* `Waypoint_Format`, `Desc_Format`

    * This determines the format used to rewrite the name and description tags in the GPX file.  If one is not specified, no change will be made for that one tag.  The name of the waypoint when you load it on your GPS is the "name" tag, and that is changed with Waypoint_Format.  Likewise, the Desc_Format changes the "desc" tag, and that is used for the waypoint description in your GPS.  The format layout supports the format codes as described below.

* `Waypoint_Max_Length`, `Desc_Max_Length`

    * If specified, the waypoint name and description will be truncated/autofit to be at most this many characters long.  If not specified the name or the description will not be truncated.

* `Waypoint_Allowed_Chars`, `Desc_Allowed_Chars`

    * Filter the waypoint name and description to only contain some characters.  If you do not include this line in your config file, no characters will be stripped.  You do not need to specify letters (a-z and A-Z), numbers (0-9), and some symbols (space and period).  Every character counts, including spaces, so "ab cd" is 5 characters: a, b, space, c, d.

* `Active_No`, `Active_Yes`, `Bug_No`, `Bug_Yes`, `Found_No`, `Found_Yes`

    * Specifies the strings used when you use the `%a`, `%b`, and `%f` format codes, depending on whether or not the specific cache is active, if it has at least one travel bug, or if you have found the cache already.

* `Found`, `Not_Found`, `TYPE_Found`, `TYPE_Not_Found`, `TYPE_SIZE_Found`, `TYPE_SIZE_Not_Found`
    * Specifies the default symbols for geocaches that are found and ones that are not found.  Rules are checked from most specific to least specific, so if you have a traditional micro cache that was not yet found, it will check for "Traditional_Micro_Not_Found", then "Traditional_Not_Found" and finally "Not_Found".  If none of those settings are in the settings file, the symbol will not change.
    * Types: Benchmark, CITO_Event, Earthcache, Event, Letterbox, Locationless, Mega, Multi, Project_APE, Traditional, Unknown, Virtual, Webcam
    * Sizes: Large, Micro, Other, Regular, Small, Unknown, Virtual


### Format Codes

Formats for waypoint names and descriptions can use the following special codes.  When you use a `%` symbol as specified below, you will get it swapped out with a different value.  You can also specify a maximum width or that this field should be automatically resized to fit into the name or description field.  See the Format Examples for an explanation.

* `%a`: This is replaced with the Active_Yes or Active_No settings if specified.  Example:  Y
* `%b`: Replaced with the Bug_Yes or Bug_No settings.  Example:  Y
* `%C`: Prefix for the cache as determine by the TYPE_Prefix settings.  Example for a traditional cache:  T
* `%D`: The difficulty of the cache.  Example: 2.5
* `%d`: The difficulty of the cache as a single number from 1 to 9 using the formula (difficulty * 2) - 1.  Example for a 2.5 cache: 4
* `%f`: "Found" indicator, which uses the Found_Yes or Found_No settings.  Example: N
* `%H` and `%h`: Hint for the geocache.  Both uppercase and lowercase will work.
* `%I`: The code after the GC in the waypoint ID.  If the waypoint doesn't start with GC, this will be the entire code.  Example for GC1234:  1234
* `%i`: The code after the first two characters in the waypoint ID.  Identical to `%I` except this always removes the first two characters.  Example for TB1234:  1234
* `%L`: First letters of the log types.  Example for "Found it", "Found it", "Didn't find it", "Found it", "Write note":  FFDFW
* `%N`: The name of the geocache.  This may change in the future to be "smart truncated".  Example:  Some Fake Cache
* `%O`: The name of the owner of the cache.  Example:  King Boreas
* `%P`: Who placed the cache.  Example:  KB & Crew
* `%p`: The first two letters of the waypoint code.  With a combination of `%p` and `%i` you have more control over splitting up the waypoint code if you want.  Example for GC1234: GC
* `%S`: The size of the container.  Example:  Small
* `%s`: The first letter of the size of the container.  Example: S
* `%T`: Terrain rating of the geocache.  Example: 3
* `%t`: Terrain rating of the geocache as a single number from 1 to 9, based on the formula (terrain * 2) - 1.  Example for a rating of 3:  5
* `%Y`: The cache type, spelled out.  Example:  Traditional Cache
* `%y`: The prefix for the cache as determined by the TYPE_Prefix settings.  Example for a traditional cache: T
* `%%`: Adds a literal %.  Example:  %
* `%0` through `%9`: Adds a number.  These are available if you wanted to add a number after the format code when you don't want to specify a length for the format code.  Example for %0: 0

### Format Modifiers

The name from `%N` is often too large to work in handheld GPS units.  You can restrict any of the above to a fixed amount of space by specifying a size after the letter code.  For instance you have 8 letters from the name by using `%N8`. 

Another special format length is 0.  It will always place in one character but will expand to fit the amount of available space.  If you had the name of  "Two Monkeys Eating Bananas" and 10 characters were allowed for the name, `%N0` would be "Two Monkey".  If your GPS showed more space and you set the max length to 20 then `%N0` would expand to "Two Monkeys Eating B".


### Format Examples

For the following examples, we will assume we are dealing with the following configuration settings and geocache.

    # This is the settings file.  It mostly uses the defaults but we override a couple settings
    Waypoint_Max_Length=10

    # A-Z, a-z, 0-9, period and space are 
    # allowed automatically if
    # we specify this value
    Waypoint_Allowed_Chars=!@#$%^&*()

    Bug_Yes=B
    Bug_No=_

And here's information about our geocache:

| Value       |                                               |
|-------------|-----------------------------------------------|
| Name        | GCTEST                                        |
| Description | It's An Example - Test                        |
| Difficulty  | 1.5                                           |
| Terrain     | 2                                             |
| Last 5 Logs | Found, Found, Did not find, Found, Write Note |
| Type        | Multi-cache                                   |
| Size        | Large                                         |
| Travel Bugs | None                                          |
| Active?     | Yes                                           |

Below are a few examples so you can understand how things are working with the format layouts.

| Where | Format | Result | Notes |
|-----|--------|--------|-------|
| Either | <none> | Unchanged | Illegal characters are also not removed. |
| Name | `%I %b` | `TEST _` | Removed GC from the waypoint ID, zero bugs in cache. | 
| Desc | `%N` | `It's An Example - Test` | Unchanged because no illegal characters were found. |
| Name | `%N` | `Its An Exa` | Illegal name character found and name shortened to max length of 10. |
| Desc | `%a%C %d=%D %t=%T` | `YT 2=1.5 3=2` | Shows mapping between difficulty and terrain rating systems. |
| Name | `%I %Y0 %s` | `TEST Tra S` | `%Y0` fills available space. |
| Desc | `%L3 - %%L3 - %L%3` | `FFD - %%L3 - FFDFW3` | Illustrating various escaping. |
| Name | `garbage in` | `garbage i` | The name was truncated to 10 characters and no format codes were found. |
| Desc | `%G%A%R%B%A%G%E out` | `%G%A%R%B%A%G%E out` | Invalid codes are copied verbatim to the output. |


### Sample Settings File

    # This is a sample settings file that shows the default settings
    # for all of the options.
    
    # This is the default code that is used when you use the %C format code.
    Benchmark_Prefix=X
    CITO_Event_Prefix=C
    Earthcache_Prefix=G
    Event_Prefix=E
    Letterbox_Hybrid_Prefix=B
    Locationless_Prefix=L
    Mega_Prefix=E
    Multi_Prefix=M
    Project_APE_Prefix=A
    Traditional_Prefix=T
    Unknown_Prefix=U
    Virtual_Prefix=V
    Webcam_Prefix=W
    
    # My own personal waypoint name and description formats, lengths,
    # and allowed characters.  This is tweaked for my own personal preferences
    # and what is allowed by my Garmin GPSMAP 60CSx.  Change for your GPS and
    # alter to what you like to see.
    Waypoint_Format=%I %s%d%t
    Waypoint_Max_Length=14
    Waypoint_Allowed_Chars=+-
    
    Desc_Format=%C%b%L
    Desc_Max_Length=30
    Desc_Allowed_Chars=+-
    
    # I want to see a B or a - in my description instead of Y/N like Active and
    # Found
    Bug_Yes=B
    Bug_No=-
    
    # The defaults for Active and Found are fine
    Active_Yes=Y
    Active_No=N
    Found_Yes=Y
    Found_No=N
    
    # Default symbols
    Found=Geocache Found
    Not_Found=Geocache_Found


Compiling
--------

These programs require the expat library, but then should compile cleanly by executing your traditional `./configure` followed by `make`.  Running `make install` copies the files to `/usr/local/bin` unless you define an alternate path with the configure command.

There is sometimes a slight hang-up with the expat library.  On one of my systems I have to include `expat.h` and the expat library (Debian).  On another, I have to include `xmlparse.h` and the xmlparse + xmltok libraries.  Since I have switched over to the automake system, I have not yet determined if this works well.  If it does not detect the expat library automatically, you should be able to use a configure option to specify where the library is located.


License
-------

The gpx_tools package is released under the GNU General Public License, as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.  See the GNU GPL [Licenses Page](http://www.gnu.org/licenses/) for more information.

In short, this is very similar to freeware - you are essentially free to use the program for whatever use you see fit.  If you are a programmer-type-person, you can also get the source code to the software and make changes to suit your will.
