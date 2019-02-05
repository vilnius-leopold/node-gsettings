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

/*
Helper function - because we do this A LOT
The returned char needs to be freed after usage.
Use 'g_free' to do so
*/
char * v8_value_to_utf8_string(const Local<Value> value_obj) {
	Local<String>  string_obj                = value_obj->ToString();
	int            utf8_string_length        = string_obj->Utf8Length();
	char          *utf8_string               = (char*) g_malloc(utf8_string_length + 1);

	string_obj->WriteUtf8(utf8_string);

	return utf8_string;
}

/* Takes schema_id string */
void get_gsetting_keys(const FunctionCallbackInfo<Value>& args) {
	GSettings *settings;
	gchar ** keys;
	gint i;
	gint size = 0;
	Isolate *isolate = args.GetIsolate();

	char *schema_id = v8_value_to_utf8_string(args[0]);

	settings = g_settings_new(schema_id);

	g_free((void *) schema_id);

	keys     = g_settings_list_keys(settings);
	size     = sizeof(keys)/sizeof(keys[0]);

	Local<Array> togo;
	togo = Array::New(isolate, size);
	for (i = 0; keys[i]; i++) {
		togo->Set(i, String::NewFromOneByte(isolate, (const uint8_t*) keys[i], NewStringType::kNormal).ToLocalChecked() );
	}
	g_strfreev(keys);
	args.GetReturnValue().Set(togo);
}

/* Takes schema_id and key */
void get_gsetting(const FunctionCallbackInfo<Value>& args) {
	GSettings          *settings;
	GVariant           *variant;
	const GVariantType *type;
	const char         *schema_id;
	const char         *key;
	Isolate *isolate = args.GetIsolate();

	schema_id = v8_value_to_utf8_string(args[0]);
	key       = v8_value_to_utf8_string(args[1]);

	// validate key and schema
	GSettingsSchemaSource *schema_source = g_settings_schema_source_get_default();
	if ( ! schema_source ) {
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*) "No schema source available!", NewStringType::kNormal ).ToLocalChecked() ));
		g_free((void *) schema_id);
		g_free((void *) key);
		args.GetReturnValue().SetUndefined();
		return;
	}
	GSettingsSchema * gsettings_schema =
		g_settings_schema_source_lookup (schema_source,
		                                 schema_id,
		                                 FALSE);
	if ( ! schema_source ) {
		isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, (const uint8_t*) "Schema is not installed!", NewStringType::kNormal ).ToLocalChecked() ));
		g_free((void *) schema_id);
		g_free((void *) key);
		args.GetReturnValue().SetUndefined();
		return;
	}
	gboolean has_key = g_settings_schema_has_key (gsettings_schema, key);
	if ( ! has_key ) {
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*) "Key does not exist!", NewStringType::kNormal ).ToLocalChecked() ));
		g_free((void *) schema_id);
		g_free((void *) key);
		args.GetReturnValue().SetUndefined();
		return;
	}

	// create varient
	settings = g_settings_new(schema_id);
	variant  = g_settings_get_value(settings,key);
	type     = g_variant_get_type(variant);

	g_free((void *) key);
	g_free((void *) schema_id);

	if (g_variant_type_equal(type,G_VARIANT_TYPE_DOUBLE)) {
		args.GetReturnValue().Set(Number::New(isolate, g_variant_get_double(variant)));
		return;
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_INT32)) {
		args.GetReturnValue().Set(Int32::New(isolate, g_variant_get_int32(variant)));
		return;
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_UINT32)) {

		const guint gunit32_value = g_variant_get_uint32(variant);

		/*
			IMPORTANT:
			Using 'Unit32::New(gunit32_value)''
			will wrong results on some architectures
			as the uint max limit can be different.
			As I understand it, v8 will try to cast
			the guint value to the architecture specific
			uint value. If the max uint value is lower
			than the one of guint, the value can be corrupted
			and can (as I experienced it) be set to -1
			To keep v8 from casting the guint
			value to a architecture dependent lower
			limited unit
			we use the more general 'Number' class
			that supports long values.
		*/
		args.GetReturnValue().Set(Number::New(isolate, gunit32_value));
		return;
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_STRING)) {
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, g_variant_get_string(variant,NULL), String::NewStringType::kNormalString));
		return;
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_STRING_ARRAY)) {
		gsize length;
		const gchar **elems = g_variant_get_strv(variant,&length);

		Local<Array> nodes = Array::New(isolate);

		for (unsigned int i = 0; i < length; ++i) {
			nodes->Set(i, String::NewFromUtf8(isolate, elems[i], String::NewStringType::kNormalString));
		}

		g_free(elems);

		args.GetReturnValue().Set(nodes);
		return;
	}
	else if (g_variant_type_equal(type, X_G_VARIANT_TYPE_STRING_TUPLE_ARRAY)) {
		Local<Array> tuple_array = Array::New(isolate);

		GVariantIter *iter;
		gchar *str1;
		gchar *str2;
		unsigned int i = 0;

		g_variant_get (variant, "a(ss)", &iter);
		while (g_variant_iter_loop (iter, "(ss)", &str1, &str2)) {
			Local<Array> tuple = Array::New(isolate);
			tuple->Set(0, String::NewFromUtf8(isolate, str1, String::NewStringType::kNormalString));
			tuple->Set(1, String::NewFromUtf8(isolate, str2, String::NewStringType::kNormalString));
			tuple_array->Set(i, tuple);
			i++;
		}
		g_variant_iter_free (iter);

		args.GetReturnValue().Set(tuple_array);
		return;
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_BOOLEAN)) {
		args.GetReturnValue().Set(Boolean::New(isolate, g_variant_get_boolean(variant)));
		return;
	}
	else {
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*)  "Need to implement reading that value type", NewStringType::kNormal ).ToLocalChecked() ));
		args.GetReturnValue().SetUndefined();
		return;
	}
}

/* Takes schema_id string */
void schema_exists(const FunctionCallbackInfo<Value>& args) {
	char                  *schema_id     = v8_value_to_utf8_string(args[0]);
	GSettingsSchemaSource *schema_source = g_settings_schema_source_get_default();
	Isolate *isolate = args.GetIsolate();

	if ( ! schema_source ) {
		g_free(schema_id);
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*)  "No schema sources available!", NewStringType::kNormal ).ToLocalChecked() ));
		args.GetReturnValue().SetUndefined();
		return;
	}

	GSettingsSchema * schema =
		g_settings_schema_source_lookup (schema_source,
		                                 schema_id,
		                                 FALSE);

	g_free(schema_id);

	if ( ! schema ) {
		args.GetReturnValue().Set(Boolean::New(isolate, false));
	} else {
		args.GetReturnValue().Set(Boolean::New(isolate, true));
	}
}

/* Takes schema_id, key, and value */
void set_gsetting(const FunctionCallbackInfo<Value>& args) {
	Local<Value>        value_obj = args[2];
	GSettings          *settings;
	GVariant           *variant;
	GVariant           *variant_to_set;
	const char         *validation_fail_message;
	const GVariantType *type;
	bool                write_success      = false;
	bool                validation_success = false;
	const char         *schema_id;
	const char         *key;
	Isolate *isolate = args.GetIsolate();

	schema_id = v8_value_to_utf8_string(args[0]);
	key       = v8_value_to_utf8_string(args[1]);

	// validate key and schema
	GSettingsSchemaSource *schema_source = g_settings_schema_source_get_default();
	if ( ! schema_source ) {
		g_free((void *) schema_id);
		g_free((void *) key);
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*) "No schema source available!", NewStringType::kNormal).ToLocalChecked() ));
		args.GetReturnValue().SetUndefined();
		return;
	}
	GSettingsSchema * gsettings_schema =
		g_settings_schema_source_lookup (schema_source,
		                                 schema_id,
		                                 FALSE);
	if ( ! schema_source ) {
		g_free((void *) schema_id);
		g_free((void *) key);
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*) "Schema is not installed!", NewStringType::kNormal).ToLocalChecked() ));
		args.GetReturnValue().SetUndefined();
		return;
	}
	gboolean has_key = g_settings_schema_has_key (gsettings_schema, key);
	if ( ! has_key ) {
		g_free((void *) schema_id);
		g_free((void *) key);
		isolate->ThrowException(Exception::Error( String::NewFromOneByte(isolate, (const uint8_t*) "Key does not exist!", NewStringType::kNormal).ToLocalChecked() ));
		args.GetReturnValue().SetUndefined();
		return;
	}
	GSettingsSchemaKey * gsettings_key =
		g_settings_schema_get_key (gsettings_schema, key);

	// get required variant type
	settings = g_settings_new(schema_id);
	variant  = g_settings_get_value(settings,key);
	type     = g_variant_get_type(variant);

	g_free((void *) schema_id);

	if ( g_variant_type_equal(type,G_VARIANT_TYPE_BOOLEAN) ) {
		if ( value_obj->IsBoolean() ) {
			validation_success = true;
			variant_to_set = g_variant_new_boolean(value_obj->BooleanValue());
		} else {
			validation_fail_message = "Key requires boolean value!";
		}
	}
	else if ( g_variant_type_equal(type,G_VARIANT_TYPE_STRING) ) {
		if ( value_obj->IsString() ) {
			validation_success = true;
			char *val = v8_value_to_utf8_string(value_obj);
			variant_to_set = g_variant_new_string((const gchar *) val);
			g_free( (void *) val );
		} else {
			validation_fail_message = "Key requires string value!";
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_DOUBLE) ) {
		if ( value_obj->IsNumber() ) {
			validation_success = true;
			variant_to_set = g_variant_new_double(value_obj->ToNumber(isolate)->Value());
		} else {
			validation_fail_message = "Key requires a number!";
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_INT32) ) {
		if ( value_obj->IsInt32() ) {
			validation_success = true;
			variant_to_set = g_variant_new_int32 (value_obj->ToInt32(isolate)->Value());
		} else {
			validation_fail_message = "Key requires a integer number!";
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_UINT32) ) {
		if ( value_obj->IsUint32() ) {
			validation_success = true;
			variant_to_set = g_variant_new_uint32( value_obj->ToUint32( isolate->GetCurrentContext() ).ToLocalChecked()->Value() );
		} else {
			validation_fail_message = "Key requires a unsigned integer number!";
		}
	}
	else if ( g_variant_type_equal(type, G_VARIANT_TYPE_STRING_ARRAY) ) {
		if ( value_obj->IsArray() ) {
			validation_success = true;
			GVariantBuilder  builder;
			uint             i;
			uint             length;
			Local<Object>    obj = value_obj->ToObject();

			// get the array length
			length = obj->Get(String::NewFromOneByte(isolate, (const uint8_t*) "length", NewStringType::kNormal).ToLocalChecked())->ToObject()->Uint32Value();

			g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);

			for(i = 0; i < length; i++)
			{
				Local<Value> element = obj->Get(i);

				if ( element->IsString() ) {
					char *val                = v8_value_to_utf8_string(element);
					GVariant *string_variant = g_variant_new_string ((const gchar *) val);
					g_free((void *) val);

					g_variant_builder_add_value (&builder, string_variant);
				}
				else {
					g_free((void *) key);
					isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, (const uint8_t*) "Array item have to be strings!", NewStringType::kNormal).ToLocalChecked()));
					args.GetReturnValue().SetUndefined();
					return;
				}
			}

			variant_to_set = g_variant_builder_end (&builder);
		} else {
			validation_fail_message = "Key requires an array of strings!";
		}
	}
	else {
		validation_fail_message = "We haven't implemented this type yet!";
	}

	if ( ! validation_success ) {
		g_free((void *) key);
		isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, (const uint8_t*) validation_fail_message, NewStringType::kNormal).ToLocalChecked()));
		args.GetReturnValue().SetUndefined();
		return;
	}

	// write variant
	gboolean is_valid = g_settings_schema_key_range_check (gsettings_key, variant_to_set);

	if ( ! is_valid ) {
		g_free((void *) key);
		isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, (const uint8_t*) "Invalid range or type!", NewStringType::kNormal).ToLocalChecked()));
		args.GetReturnValue().SetUndefined();
		return;
	}

	write_success = g_settings_set_value(settings, key, variant_to_set);

	g_settings_sync();

	g_free((void *) key);

	if ( ! write_success ) {
		isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, (const uint8_t*) "Failed to set gsetting! Key is write protected.", NewStringType::kNormal).ToLocalChecked()));
	}

	args.GetReturnValue().SetUndefined();
	return;
}

/* in addon initialization function */
void init(Local<Object> target) {
	Isolate *isolate = target->GetIsolate();

	target->Set(String::NewFromOneByte(isolate, (const uint8_t*) "set_gsetting", NewStringType::kNormal).ToLocalChecked(),
	            FunctionTemplate::New(isolate, set_gsetting)->GetFunction());
	target->Set(String::NewFromOneByte(isolate, (const uint8_t*) "get_gsetting", NewStringType::kNormal).ToLocalChecked(),
	            FunctionTemplate::New(isolate, get_gsetting)->GetFunction());
	target->Set(String::NewFromOneByte(isolate, (const uint8_t*) "get_gsetting_keys", NewStringType::kNormal).ToLocalChecked(),
	            FunctionTemplate::New(isolate, get_gsetting_keys)->GetFunction());
	target->Set(String::NewFromOneByte(isolate, (const uint8_t*) "schema_exists", NewStringType::kNormal).ToLocalChecked(),
	            FunctionTemplate::New(isolate, schema_exists)->GetFunction());
}

NODE_MODULE(nodegsettings, init)
