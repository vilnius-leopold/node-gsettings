var gSettingsBinding = require('./build/Release/nodegsettings.node');

// Constructor
function GSettings( schemaId ) {
	if ( ! schemaId.match(/^([a-zA-z0-9\-_]+\.)+[a-zA-z0-9\-_]+$/) )
		throw new Error("'"+schemaId+"' is not a valid schema id!");

	if ( ! gSettingsBinding.schema_exists(schemaId) )
		throw new Error("Shema '"+schemaId+"' is not installed!");

	this.schemaId = schemaId;
}

// Instance methods
GSettings.prototype.get = function( key ) {
	if ( ! key.match(/^[a-z0-9\-]+$/) )
		throw new Error("Invalid key name format '"+key+"' !");

	return gSettingsBinding.get_gsetting( this.schemaId, key );
};

GSettings.prototype.set = function( key, value ) {
	if ( ! key.match(/^[a-z0-9\-]+$/) )
		throw new Error("Invalid key name format '"+key+"' !");

	if ( typeof value === 'undefined' )
		throw new Error("Value is undefined!");

	gSettingsBinding.set_gsetting( this.schemaId, key, value );
};

GSettings.prototype.getAll = function() {
	var keyList  = this.getKeyList(),
	    keyCount = keyList.length,
	    settings = {},
	    value,
	    key;

	for ( var i = 0; i < keyCount; i++ ) {
		key   = keyList[i];
		value = this.get( key );

		settings[key] = value;
	}

	return settings;
};

GSettings.prototype.getKeyList = function() {
	return gSettingsBinding.get_gsetting_keys( this.schemaId );
};

GSettings.prototype.serialize = function() {
	return JSON.stringify( this.getAll() );
};

// Class methods
GSettings.getSchemaList = function() {
	throw "This method has not been implemented yet!";
};

module.exports = GSettings;