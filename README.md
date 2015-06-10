node-gsettings
==============
Nodejs binding to [gsettings](https://developer.gnome.org/gio/stable/GSettings.html). Gsettings is a common way of storing application configurations on linux in a standardized, transparent manner. Gsettings uses [dconf](https://wiki.gnome.org/action/show/Projects/dconf) for its backend.

This project is a fork of [GPII/linux gsettingsBridge](https://github.com/GPII/linux/tree/master/gpii/node_modules/gsettingsBridge).


## Install
```sh
npm install node-gsettings
```

## API
Currently the API is very limited to very basic gsettings functionality. It is intended for updating and reading application settings. This is not an API for settings up or configuring dconf settings.


### Implemented Types
dconf has a very flexible [type system](https://developer.gnome.org/glib/stable/glib-GVariantType.html). Currently only the following are implemented:

| GVariant Type |  Javascript Type      | Javascript Example      | Read | Write |
| ------------- | --------------------- | ----------------------- | :--: | :---: |
| `b`           | Boolean               | `true`, `false`             |  ✓   |   ✓   |
| `s`           | String                | `"Some string"`           |  ✓   |   ✓   |
| `i`, `u`, `d` | Number                | `-12`, `5`, `3.12`            |  ✓   |   ✓   |
| `as`          | Array of Strings      | `[ 'one', 'two', 'three' ]` |  ✓   |   ✓   |
| `a(ss)`       | Array of String Pairs | `[ ['one', 'two'], ['three', 'four'] ]` |  ✓   |       |


### Usage
```javascript
var GSettings = require("node-gsettings");

var schemaId = 'org.gnome.desktop.interface';

// create a new settings object
var settings = new GSettings( schemaId );

// get single property
var clockFormat = settings.get('clock-format');

// get all properties
var settingsData = settings.getAll();

// get serialized properties
var settingsJSON = settings.serialize();

// get list of keys
var keyList = settings.getKeyList();

// setting a single key
settings.set('clock-format', '12h');
```

## Contributers
- [Antranig Basman](https://github.com/amb26)
- [Leopold Burdyl](https://github.com/vilnius-leopold)

## License
[BSD-3-Clause](LICENSE.txt)