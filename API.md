API Proposal
============

```
var gSettings = require('node-gsettings');

// Global methods
var schemaNameList = gSettings.getSchemaList();
var settingsObjArray = gSettings.getAllSettings();

// Schema interaction
// support of two styles

// 1) Direct access
	// get keys
	var keys = gSettings.getKeys('/org/my/schema');

	// get value
	var value = gSettings.get('/org/my/schema', 'key');

	// set value
	gSettings.set('/org/my/schema', 'key', value);

// 2) OO factory style
	// get schema
	var mySchema = gSettings.getSettings('/org/my/schema');

	console.log( mySchema.name ); // '/org/my/schema'

	// get keys
	var keys    = mySchema.getKeys();
	var keyList = mySchema.getKeyList();

	// get value
	var value = mySchema.get('key');

	var values = mySchema.getAll();
	/*
		values = {
			'key1': value1,
			'key2': ['A', 'B', 'C'],
			'key3': 1234
		}

	*/
	var schemaJSON = mySchema.serialize();

	// set value
	mySchema.set('key', value);
	mySchema.set({
		'key1': value1,
		'key2': ['A', 'B', 'C'],
		'key3': 1234
	});


// 3) OO style with 'new'

var GSettings = require('node-gsettings');

// Class methods
GSettings.getSchemaList();

var settings = new GSettings('/org/my/schema');

// Instance methods
var keyValue     = settings.get('key');
var allKeyValues = settings.getAll();

settings.set('key', value);
settings.set({
	'key1': value1,
	'key2': ['A', 'B', 'C'],
	'key3': 1234
});

settings.getKeyList();
settings.serialize();


```