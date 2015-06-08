var assert    = require('assert'),
    GSettings = require("./../index.js");

var schemaId = 'org.gnome.desktop.interface',
    settings = new GSettings( schemaId );

console.log('schemaId', settings.schemaId);

assert( settings.schemaId, schemaId);

assert( settings.get('clock-format'), '24h');
settings.set('clock-format', '12h');
assert( settings.get('clock-format'), '12h');
settings.set('clock-format', '24h');
assert( settings.get('clock-format'), '24h');

// console.log( 'Key list: ', settings.getKeyList() );

var settingsData = settings.getAll();
var settingsData = new GSettings('org.gnome.desktop.input-sources').getAll();

var settings2 = new GSettings('org.gnome.desktop.search-providers');

settings2.getAll();
var sortOrder = settings2.get('sort-order');

console.log('sort-order', sortOrder);
settings2.set('sort-order', sortOrder);
// var settingsData = new GSettings('org.gnome.Totem').getAll();


assert( Object.keys(settingsData).length, settings.getKeyList().length);

// console.log( settingsData );