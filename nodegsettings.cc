/*
 * GPII Node.js GSettings Bridge
 *
 * Copyright 2012 Steven Githens
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * The research leading to these results has received funding from the European Union's
 * Seventh Framework Programme (FP7/2007-2013)
 * under grant agreement no. 289016.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 *
 */

/*
 * For changes done after initial fork from
 * https://github.com/GPII/linux
 * based on commit b28e99212e4c7cc803f210c066e0124d72c8b72d.
 *
 * Copyright 2015 Leopold Burdyl
 */

#include <node.h>
#include <v8.h>
#include <gio/gio.h>

using namespace v8;

// globals
const GVariantType* X_G_VARIANT_TYPE_STRING_TUPLE_ARRAY = g_variant_type_new("a(ss)");


/* Helper function - because we do this A LOT */
char * v8_value_to_utf8_string(const Local<Value> value_obj) {
	Local<String>  string_obj                = value_obj->ToString();
	int            utf8_string_length        = string_obj->Utf8Length();
	char          *utf8_string               = (char*) g_malloc(utf8_string_length + 1);

	string_obj->WriteUtf8(utf8_string);

	return utf8_string;
}

/* Takes schemaId string */
Handle<Value> get_gsetting_keys(const Arguments& args) {
	HandleScope scope;
	GSettings *settings;
	gchar ** keys;
	gint i;
	gint size = 0;

	char *schema = v8_value_to_utf8_string(args[0]);

	settings = g_settings_new(schema);
	keys     = g_settings_list_keys(settings);
	size     = sizeof(keys)/sizeof(keys[0]);

	Handle<Array> togo;
	togo = Array::New(size);
	for (i = 0; keys[i]; i++) {
		togo->Set(i,String::New(keys[i]));
	}
	return scope.Close(togo);
}

/* Takes schemaId and key */
Handle<Value> get_gsetting(const Arguments& args) {
	HandleScope         scope;
	GSettings          *settings;
	GVariant           *variant;
	const GVariantType *type;
	const char         *schema;
	const char         *key;

	schema   = v8_value_to_utf8_string(args[0]);
	key      = v8_value_to_utf8_string(args[1]);

	// validate key and schema
	GSettingsSchemaSource *schema_source = g_settings_schema_source_get_default();
	if ( ! schema_source ) {
		ThrowException(Exception::Error(String::New("No schema source available!")));
		return scope.Close(Undefined());
	}
	GSettingsSchema * gsettings_schema =
		g_settings_schema_source_lookup (schema_source,
		                                 schema,
		                                 FALSE);
	if ( ! schema_source ) {
		printf("Schema '%s' is not installed!\n", schema);
		ThrowException(Exception::Error(String::New("Schema is not installed!")));
		return scope.Close(Undefined());
	}
	gboolean has_key = g_settings_schema_has_key (gsettings_schema, key);
	if ( ! has_key ) {
		printf("Key '%s' does not exist!\n", key);
		ThrowException(Exception::Error(String::New("Key does not exist!")));
		return scope.Close(Undefined());
	}

	// create varient
	settings = g_settings_new(schema);
	variant  = g_settings_get_value(settings,key);
	type     = g_variant_get_type(variant);

	if (g_variant_type_equal(type,G_VARIANT_TYPE_DOUBLE)) {
		return scope.Close(Number::New(g_variant_get_double(variant)));
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_INT32)) {
		return scope.Close(Int32::New(g_variant_get_int32(variant)));
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_UINT32)) {

		const guint gunit32_value = g_variant_get_uint32(variant);

		/*
			IMPORTANT:
			Using 'Unit32::New(gunit32_value)''
			will wrong results on some architectures
			as the uint max limit can be different.
			As I understand it, v8 will try to cast
			the guint value to the achitecture specific
			uint value. If the max uint value is lower
			than the one of guint, the value can be corrupted
			and can (as I experienced it) be set to -1
			To keep v8 from casting the guint
			value to a achitecutre dependent lower
			limited unit
			we use the more general 'Number' class
			that supports long values.
		*/
		return scope.Close(Number::New(gunit32_value));
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_STRING)) {
		return scope.Close(String::New(g_variant_get_string(variant,NULL)));
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_STRING_ARRAY)) {
		gsize length;
		const gchar **elems = g_variant_get_strv(variant,&length);

		HandleScope scope;
		Local<Array> nodes = Array::New();

		for (unsigned int i = 0; i < length; ++i) {
			nodes->Set(i, String::New(elems[i]));
		}

		return scope.Close(nodes);
	}
	else if (g_variant_type_equal(type, X_G_VARIANT_TYPE_STRING_TUPLE_ARRAY)) {
		Local<Array> tuple_array = Array::New();

		GVariantIter *iter;
		gchar *str1;
		gchar *str2;
		unsigned int i = 0;

		g_variant_get (variant, "a(ss)", &iter);
		while (g_variant_iter_loop (iter, "(ss)", &str1, &str2)) {
			Local<Array> tuple = Array::New();
			tuple->Set(0, String::New(str1));
			tuple->Set(1, String::New(str2));
			tuple_array->Set(i, tuple);
			i++;
		}
		g_variant_iter_free (iter);

		return scope.Close(tuple_array);
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_BOOLEAN)) {
		return scope.Close(Boolean::New(g_variant_get_boolean(variant)));
	}
	else {
		g_print("The type is %s\n", g_variant_type_peek_string(type));
		ThrowException(Exception::Error(String::New("Need to implement reading that value type")));
		return scope.Close(Undefined());
	}
}

/* Takes schemaId string */
Handle<Value> schema_exists(const Arguments& args) {
	HandleScope            scope;
	char                  *schema_id     = v8_value_to_utf8_string(args[0]);
	GSettingsSchemaSource *schema_source = g_settings_schema_source_get_default();

	if ( ! schema_source ) {
		ThrowException(Exception::Error(String::New("No schema sources available!")));
		return scope.Close(Undefined());
	}

	GSettingsSchema * schema =
		g_settings_schema_source_lookup (schema_source,
		                                 schema_id,
		                                 FALSE);

	if ( ! schema ) {
		return scope.Close(Boolean::New(FALSE));
	}

	return scope.Close(Boolean::New(TRUE));
}

/* Takes schemaId, key, and value */
Handle<Value> set_gsetting(const Arguments& args) {
	HandleScope         scope;
	Local<Value>        value_obj = args[2];
	GSettings          *settings;
	GVariant           *variant;
	const GVariantType *type;
	bool                success = false;
	const char         *schemaId;
	const char         *key;

	schemaId = v8_value_to_utf8_string(args[0]);
	key      = v8_value_to_utf8_string(args[1]);

	// validate key and schema
	GSettingsSchemaSource *schema_source = g_settings_schema_source_get_default();
	if ( ! schema_source ) {
		ThrowException(Exception::Error(String::New("No schema source available!")));
		return scope.Close(Undefined());
	}
	GSettingsSchema * gsettings_schema =
		g_settings_schema_source_lookup (schema_source,
		                                 schemaId,
		                                 FALSE);
	if ( ! schema_source ) {
		printf("Schema '%s' is not installed!\n", schemaId);
		ThrowException(Exception::Error(String::New("Schema is not installed!")));
		return scope.Close(Undefined());
	}
	gboolean has_key = g_settings_schema_has_key (gsettings_schema, key);
	if ( ! has_key ) {
		printf("Key '%s' does not exist!\n", key);
		ThrowException(Exception::Error(String::New("Key does not exist!")));
		return scope.Close(Undefined());
	}
	GSettingsSchemaKey * gsettings_key =
		g_settings_schema_get_key (gsettings_schema, key);

	// get required variant type
	settings = g_settings_new(schemaId);
	variant  = g_settings_get_value(settings,key);
	type     = g_variant_get_type(variant);


	if ( g_variant_type_equal(type,G_VARIANT_TYPE_BOOLEAN) ) {
		if ( value_obj->IsBoolean() ) {
			success = g_settings_set_boolean(settings,key,value_obj->BooleanValue());
		} else {
			ThrowException(Exception::Error(String::New("Key requires boolean value!")));
			return scope.Close(Undefined());
		}
	}
	else if ( g_variant_type_equal(type,G_VARIANT_TYPE_STRING) ) {
		if ( value_obj->IsString() ) {
			char *val = v8_value_to_utf8_string(value_obj);

			success = g_settings_set_string(settings,key,val);
		} else {
			ThrowException(Exception::Error(String::New("Key requires string value!")));
			return scope.Close(Undefined());
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_DOUBLE) ) {
		if ( value_obj->IsNumber() ) {
			gdouble v8_double_value = value_obj->ToNumber()->Value();
			GVariant *variant_to_set = g_variant_new_double(v8_double_value);

			gboolean is_valid = g_settings_schema_key_range_check (gsettings_key, variant_to_set);
			if ( ! is_valid ) {
				printf("Invalid range or type of '%s'\n", key);
				ThrowException(Exception::Error(String::New("Invalid range or type!")));
				return scope.Close(Undefined());
			}

			success = g_settings_set_value(settings, key, variant_to_set);
		} else {
			ThrowException(Exception::Error(String::New("Key requires a number!")));
			return scope.Close(Undefined());
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_INT32) ) {
		if ( value_obj->IsInt32() ) {
			GVariant *variant_to_set = g_variant_new_int32 (value_obj->ToInt32()->Value());

			gboolean is_valid = g_settings_schema_key_range_check (gsettings_key, variant_to_set);

			if ( ! is_valid ) {
				printf("Invalid range or type of '%s'\n", key);
				ThrowException(Exception::Error(String::New("Invalid range or type!")));
				return scope.Close(Undefined());
			}

			success = g_settings_set_value(settings, key, variant_to_set);
		} else {
			ThrowException(Exception::Error(String::New("Key requires a integer number!")));
			return scope.Close(Undefined());
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_UINT32) ) {
		if ( value_obj->IsUint32() ) {
			GVariant *variant_to_set = g_variant_new_uint32(value_obj->ToUint32()->Value());

			gboolean is_valid = g_settings_schema_key_range_check (gsettings_key, variant_to_set);

			if ( ! is_valid ) {
				printf("Invalid range or type of '%s'\n", key);
				ThrowException(Exception::Error(String::New("Invalid range or type!")));
				return scope.Close(Undefined());
			}

			success = g_settings_set_value(settings, key, variant_to_set);
		} else {
			ThrowException(Exception::Error(String::New("Key requires a unsigned integer number!")));
			return scope.Close(Undefined());
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_STRING_ARRAY) ) {
		if ( value_obj->IsArray() ) {
			GVariantBuilder  builder;
			GVariant        *string_array_varient;
			uint             i;
			uint             length;
			Local<Object>    obj = value_obj->ToObject();

			// get the array length
			length = obj->Get(String::New("length"))->ToObject()->Uint32Value();

			g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);

			for(i = 0; i < length; i++)
			{
				Local<Value> element = obj->Get(i);

				if ( element->IsString() ) {
					char *val                = v8_value_to_utf8_string(element);
					GVariant *string_variant = g_variant_new_string ((const gchar *) val);

					g_variant_builder_add_value (&builder, string_variant);
				}
				else {
					ThrowException(Exception::Error(String::New("Array item have to be strings!")));
					return scope.Close(Undefined());
				}
			}

			string_array_varient = g_variant_builder_end (&builder);

			success = g_settings_set_value(settings, key, string_array_varient);
		} else {
			ThrowException(Exception::Error(String::New("Key requires an array of strings!")));
			return scope.Close(Undefined());
		}
	}
	else {
		ThrowException(Exception::Error(String::New("We haven't implemented this type yet!")));
	}

	g_settings_sync();

	if ( ! success ) {
		ThrowException(Exception::Error(String::New("Failed to set gsetting!")));
	}

	return scope.Close(Undefined());
}

/* in addon initialization function */
void init(Handle<Object> target) {
	setvbuf(stdout, NULL, _IONBF, 0);

	target->Set(String::NewSymbol("set_gsetting"),
	            FunctionTemplate::New(set_gsetting)->GetFunction());
	target->Set(String::NewSymbol("get_gsetting"),
	            FunctionTemplate::New(get_gsetting)->GetFunction());
	target->Set(String::NewSymbol("get_gsetting_keys"),
	            FunctionTemplate::New(get_gsetting_keys)->GetFunction());
	target->Set(String::NewSymbol("schema_exists"),
	            FunctionTemplate::New(schema_exists)->GetFunction());
}

NODE_MODULE(nodegsettings, init)