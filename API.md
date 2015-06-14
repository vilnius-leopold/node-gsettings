API Proposal
============
## Use Cases
- **Application Administration** (e.g. GPII): updating multiple settings of multiple (other) applications
- **Application Settings** (e.g. my app): read/update/monitor applications's own settings


## To throw or not to throw

## Module Styles
[Interesting read-up concerning nodejs module design](http://psteeleidem.com/node-module-factory-pattern/)

### 1) Module style - Namespaced Collection of functions
```javascript
var gSettings = require('node-gsettings');

// get keys
var keys = gSettings.getKeys('/org/my/schema');

// get value
var value = gSettings.get('/org/my/schema', 'key');

// set value
gSettings.set('/org/my/schema', 'key', value);
```
- PRO:
	- straightforward usage
	- flexible when working with many different schemas
	- close to native nodejs modules design (e.g. `fs` module)
- CON:
	- inconvenient when working with only one schema (always have to add schemaId for every operation)
	- does not promote OO view on gsettings


### 2) Factory style - Module with factory method
```javascript
// module Object
var ngs = require('node-gsettings');

// module methods
var schemaNameList   = ngs.getSchemaList();

// can also hold module methods of 1)
// var value = ngs.get('/org/my/schema', 'key');
// ngs.set('/org/my/schema', 'key', value);
// ...

// Factory method creates Class:GSettings instance object
var settings = ngs.createGSettings('/org/my/schema');

// is equal to
// var settings = new ngs.GSettings('/org/my/schema');

// GSettings instance methods
// get keys
var keys    = settings.getKeys();
var keyList = settings.getKeyList();

// get value
var value = settings.get('key');

var values = settings.getAll();
/*
    values = {
        'key1': value1,
        'key2': ['A', 'B', 'C'],
        'key3': 1234
    }

*/
var schemaJSON = settings.serialize();

// set value
settings.set('key', value);
settings.setMultiple({
    'key1': value1,
    'key2': ['A', 'B', 'C'],
    'key3': 1234
});
```
- PRO:
	- can combine the best of both worlds, OO and module style
- CON:
	- having two ways to do the same thing can be confusing/strange



### 3) OO Style - Exporting a Class Object
```javascript
// Class object
var GSettings = require('node-gsettings');

// Class methods
GSettings.getSchemaList();

// Create Instance with 'new' operator
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

- PRO:
	- gsettings fits into OO view
	- you get the benefits of OO:
		- encapsulated/efficient/clean when mainly operating on a single gSettings object (no need to pass in schemaId each time)
- CON:
	- awkward when having to do single API calls on many different schemas
	- using 'new' on a module object is awkward
	- not consistent with native nodejs module style