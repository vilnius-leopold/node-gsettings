var assert    = require('assert'),
    GSettings = require("./../index.js");

// create object
var schemaId = 'org.gnome.desktop.interface',
    settings = new GSettings( schemaId );

// schema id test
assert( settings.schemaId, schemaId);

// string set test
assert( settings.get('clock-format'), '24h');
settings.set('clock-format', '12h');
assert( settings.get('clock-format'), '12h');
settings.set('clock-format', '24h');
assert( settings.get('clock-format'), '24h');

// getAll test
// read Uint32
var settingsData = settings.getAll();

// serialize test
var settingsJSON = settings.serialize();

// read array of string tuple
var settingsData2 = new GSettings('org.gnome.desktop.input-sources').getAll();

// read string array
var settings2 = new GSettings('org.gnome.desktop.search-providers');
settings2.getAll();

// string array write test
var originalSortOrder = settings2.get('sort-order');
console.log('ORIGINAL sort-order', originalSortOrder);

settings2.set('sort-order', originalSortOrder.reverse());
var reversedSortOrder = settings2.get('sort-order');
console.log('REVERSE sort-order:', reversedSortOrder);
// assert(originalSortOrder.reverse(), reversedSortOrder);

settings2.set('sort-order', reversedSortOrder.reverse());
var restoredSortOrder = settings2.get('sort-order');
console.log('RESTORED sort-order:', restoredSortOrder);
// assert.deepEqual(originalSortOrder, restoredSortOrder);

// var settingsData2 = new GSettings('org.gnome.Totem').getAll();

// test key list
assert( Object.keys(settingsData2).length, settings.getKeyList().length);

console.log( "TESTS DONE!" );