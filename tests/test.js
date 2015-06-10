var assert    = require('assert'),
    GSettings = require("./../index.js");

// Load test data
var testData = JSON.parse( require('fs').readFileSync( 'tests/testData.json', 'utf8') );

function getTestData( settings ) {
	var obj = {};

	testData[settings.schemaId].forEach(function(data) {
		obj[data.key] = data.originalValue;
	});

	return obj;
}

var instanceFunctionTests = [
	{
		name: 'get',
		args: ['string-setting'],
		failArgs:[
			[],
			[undefined],
			[null],
			[1234]
		],
		expectedResult: 'abcdefg'
	},{
		name: 'set',
		testFunction: function(settings, funcName) {
			var origValue = settings.get('string-setting');
			var testValue = 'test test';
			settings[funcName]('string-setting', testValue);
			var appliedValue = settings.get('string-setting');

			assert(testValue,appliedValue);
			settings.set('string-setting', origValue);
			appliedValue = settings.get('string-setting');
			assert(origValue,appliedValue);
		}
	},{
		name: 'getAll',
		testFunction: function(settings, funcName) {
			assert.deepEqual( settings[funcName](), getTestData(settings) );
		}
	},{
		name: 'serialize',
		testFunction: function(settings, funcName) {
			assert.deepEqual(
				JSON.parse(settings[funcName]()),
				getTestData(settings)
			);
		}
	},{
		name: 'getKeyList',
		testFunction: function(settings, funcName) {
			assertHasSameItems( settings[funcName](), getTestKeys(settings) );
		}

	}
];

function testInstanceFunctions( settings ) {
	var instanceFunctions = [],
	    testedFunctions   = [];

	for ( var name in settings ) {
		var attr = settings[name];

		if ( typeof attr === 'function' )
			instanceFunctions.push( name );
	}

	instanceFunctionTests.forEach(function( data ) {
		testedFunctions.push(data.name);
	});

	assertHasSameItems(instanceFunctions, testedFunctions);

	instanceFunctionTests.forEach(function( data ) {
		if ( data.skipTest ) {
			console.warn('WARN: Skipping function test for \'' + data.name + "'")
			return true;
		}

		console.log("Testing function '" + data.name + "'");

		if ( data.expectedResult ) {
			var func = settings[data.name];

			assert.deepEqual(
				func.apply(settings, data.args),
				data.expectedResult
			);
		} else if ( data.testFunction ) {
			data.testFunction(settings, data.name);
		} else {
			throw "Missing test implementation!";
		}
	});
}

function getTestKeys(settings) {
	var keys = [];

	testData[settings.schemaId].forEach(function( data ){
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


	testInstanceFunctions( settings );

	// must throw error on
	// non-existing schema
	console.log('Testing fake schema error handing');
	assert.throws(function() {
		new GSettings( schemaId + '.fake-schema' );
	});

	assert.throws(function() {
		new GSettings( '?random!/format.strange84string' );
	});

	assert.throws(function() {
		new GSettings( 345234 );
	});


	// test instance properties
	assert( settings.schemaId, schemaId);

	// test if tests for all keys are implemented
	// and if key list returned is correct

	// need to throw error
	// on non existing key
	assert.throws(function() {
		settings.get('INvalid?key-format!');
	});
	// assert.throws(function() {
	// 	settings.get('fake-key');
	// });

	// assert.throws(function() {
	// 	settings.set('fake-key', 'fake-value');
	// });

	schemaTestData.forEach(function( data ) {
		if ( data.skipTest ) {
			console.warn('WARN: Skipping test for \'' + data.key + '\'');
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

		// FAIL tests
		// if (data.failTestValues) {
		// 	console.log('    FAIL test values');
		// 	data.failTestValues.forEach(function( testValue ){
		// 		console.log('        Fail Value   : ', testValue);

		// 		assert.throws(function(){
		// 			settings.set( data.key, testValue );
		// 		var appliedValue = settings.get( data.key );
		// 		console.log('        Applied Value: ', appliedValue);
		// 		});

		// 		assert.deepEqual( appliedValue, testValue );
		// 	});
		// } else {
		// 	console.warn('WARN: Missing fail tests');
		// }

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
