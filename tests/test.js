var assert    = require('assert'),
    GSettings = require("./../index.js");

var testData = {
	'com.github.vilnius-leopold.node-gsettings.testing': [
		{
			key: 'boolean-setting',
			originalValue: false,
			// skipTest: true,
			testValues: [true],
			failTestValues: ['string',null]
		},{
			key: 'unsigned-integer-setting',
			originalValue: 15,
			testValues: [9134534],
			failTestValues: [null, -12]
		},{
			key: 'integer-setting',
			originalValue: -15,
			testValues: [2312,-3423],
			failTestValues: [0.12]
		},{
			key: 'double-setting',
			originalValue: 5.0,
			testValues: [-12.346, 6.9999999999],
			failTestValues: []
		},{
			key: 'string-setting',
			originalValue: "abcdefg",
			testValues: ["Try super long", "Try UTF8 strange charsöä PÜÖ,''ūė-čėą“"],
			failTestValues: [null, 5]
		},{
			key: 'string-array-setting',
			originalValue: ["one", "two", "three"],
			testValues: [
				["adfasd", "12dfs", "xyz"]
			],
			failTestValues: []
		},{
			key: 'string-tuple-array-setting',
			originalValue: [["one", "two"], ["three", "four"]],
			testValues: [
				[["x", "y"], ["abc", "efg"]]
			],
			skipTest: true,
			failTestValues: [
				[["x"], ["abc", "efg"]],
				[[1,2], ["abc", "efg"]]
			]
		},
	]
};

function getTestKeys(schemaTestData) {
	var keys = [];

	schemaTestData.forEach(function( data ){
		keys.push( data.key );
	});

	return keys;
}

function assertHasSameItems( a1, a2 ) {
	assert(a1.length, a2.length);

	a1.forEach(function(key){
		assert.notEqual(a2.indexOf(key), -1, "Missing key '" + key + "'" );
	});
}

function runAllTests() {
	for ( var schemaId in testData ) {
		var schemaTestData = testData[schemaId];

		runTest(schemaId, schemaTestData );
	}

	console.log( "TESTS DONE!" );
}

function runTest( schemaId, schemaTestData ) {

	var settings = new GSettings( schemaId );

	// test schema id
	assert( settings.schemaId, schemaId);

	// test if tests for all keys are implemented
	// and if key list returned is correct
	assertHasSameItems( settings.getKeyList(), getTestKeys(schemaTestData) );

	schemaTestData.forEach(function( data ) {
		if ( data.skipTest ) {
			console.warn('WARN: Skipping test for \'' + data.key + '\'')
			return true;
		}

		console.log('Testing key \'' + data.key + '\'');

		// READ default test
		console.log('    READ default value');
		var defaultValue = data.originalValue;
		console.log( '        Expected Value: ', defaultValue );
		var actualValue   = settings.get(data.key);

		console.log( '        Actual Value  : ', actualValue );
		assert.deepEqual( defaultValue, actualValue, "Maybe schema was not reset?" );

		// WRITE test
		console.log('    WRITE test values');
		data.testValues.forEach(function( testValue ){
			console.log('        Test Value   : ', testValue);

			settings.set( data.key, testValue );
			var appliedValue = settings.get( data.key );
			console.log('        Applied Value: ', appliedValue);
			assert.deepEqual( appliedValue, testValue );
		});

		// RESTORE default
		console.log('    RESTORE default value');
		console.log('        Default Value: ', defaultValue);

		settings.set( data.key, defaultValue );
		var appliedValue = settings.get( data.key );
		console.log('        Applied Value: ', appliedValue);
		assert.deepEqual( defaultValue, appliedValue );

		console.log('    Pass tests for key \'' + data.key + '\'');
	});

	console.log( "All tests pass for schema '"+ schemaId +"'" );
}

runAllTests();
