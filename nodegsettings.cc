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
 */

// #include <stdio.h>
#include <node.h>
#include <v8.h>

#include <gio/gio.h>

using namespace v8;

// in globals
// static Persistent<String> data_symbol;
// static Persistent<String> tag_symbol;
const GVariantType* X_G_VARIANT_TYPE_STRING_TUPLE_ARRAY = g_variant_type_new("a(ss)");



Handle<Value> get_gsetting_keys(const Arguments& args) {
	HandleScope scope;
	GSettings *settings;
	char schema[1024];
	gchar ** keys;
	gint i;
	gint size = 0;
	args[0]->ToString()->WriteAscii(schema);
	settings = g_settings_new(schema);
	keys = g_settings_list_keys(settings);
	for (i = 0; keys[i]; i++) {
		size++; // Figure out how to do this in 1 loop
	}
	Handle<Array> togo;
	togo = Array::New(size);
	for (i = 0; keys[i]; i++) {
		togo->Set(i,String::New(keys[i]));
	}
	return scope.Close(togo);
}

/* Should take schema and key */
Handle<Value> get_gsetting(const Arguments& args) {
	HandleScope scope;
	GSettings *settings;
	char schema[1024];
	char key[1024];
	args[0]->ToString()->WriteAscii(schema);
	settings = g_settings_new(schema);
	args[1]->ToString()->WriteAscii(key);
	GVariant* variant;
	const GVariantType* type;

	variant = g_settings_get_value(settings,key);
	type = g_variant_get_type(variant);

	setbuf(stdout, NULL);

	if (g_variant_type_equal(type,G_VARIANT_TYPE_DOUBLE)) {
		return scope.Close(Number::New(g_variant_get_double(variant)));
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_INT32)) {
		return scope.Close(Int32::New(g_variant_get_int32(variant)));
	}
	else if (g_variant_type_equal(type,G_VARIANT_TYPE_UINT32)) {
		return scope.Close(Uint32::New(g_variant_get_uint32(variant)));
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
		HandleScope scope;
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

/* Should take schema, key, and value */
Handle<Value> set_gsetting(const Arguments& args) {
	HandleScope scope;
	GSettings *settings;
	GVariant* variant;
	const GVariantType* type;
	char schema[1024];
	char key[1024];
	bool status = false;

	args[0]->ToString()->WriteAscii(schema);
	settings = g_settings_new(schema);
	args[1]->ToString()->WriteAscii(key);
	if (args[2]->IsBoolean()) {
		status = g_settings_set_boolean(settings,key,args[2]->BooleanValue());
	}
	else if (args[2]->IsNumber()) {
		variant = g_settings_get_value(settings,key);
		type = g_variant_get_type(variant);
		if (g_variant_type_equal(type,G_VARIANT_TYPE_DOUBLE)) {
			status = g_settings_set_double(settings,key,args[2]->ToNumber()->Value());
		}
		else if (g_variant_type_equal(type,G_VARIANT_TYPE_INT32)) {
			status = g_settings_set_int(settings,key,args[2]->ToInt32()->Value());
		}
		else {
			g_print("The type is %s\n", g_variant_type_peek_string(type));
			ThrowException(Exception::Error(String::New("We haven't implemented this number type yet!")));
		}
	}
	else if (args[2]->IsString()) {
		char val[1024];
		variant = g_settings_get_value(settings,key);
		type = g_variant_get_type(variant);
		args[2]->ToString()->WriteAscii(val);
		status = g_settings_set_string(settings,key,val);
	}
	else if (args[2]->IsArray()) {

		variant = g_settings_get_value(settings,key);

		type = g_variant_get_type(variant);

		Local<Object> obj = args[2]->ToObject();

		// get the array length
		int length = obj->Get(String::New("length"))->ToObject()->Uint32Value();

		// gchar array
		GVariant *string_variant_array[length];
		gchar *string_array[length];

		if ( g_variant_type_equal(type, G_VARIANT_TYPE_STRING_ARRAY) ) {
			// now that we know that gsetting requires an array of strings
			// we have to check if the v8 object we received actually
			// only contains string

			int i;
			GVariantBuilder builder;
			g_variant_builder_init (&builder, G_VARIANT_TYPE_STRING_ARRAY);

			for(i = 0; i < length; i++)
			{
					Local<Value> element = obj->Get(i);

					if ( element->IsString() ) {
						Local<String> string_obj = element->ToString();
						const int string_length = string_obj->Length();

						gchar val[string_length+1];

						string_obj->WriteUtf8(val);

						GVariant *string_variant = g_variant_new_string ((const gchar *) val);

						g_variant_builder_add_value (&builder, string_variant);
					}
					else {
						ThrowException(Exception::Error(String::New("Invalid Array format!")));
					}
			}

			GVariant * string_array_varient = g_variant_builder_end (&builder);

			int success = g_settings_set_value(settings, key, string_array_varient);

			if ( ! success ) {
				ThrowException(Exception::Error(String::New("Updating string array failed!")));
			}
		}
		// else if ( g_variant_type_equal(type, X_G_VARIANT_TYPE_STRING_TUPLE_ARRAY) ) {

		// }
		else {
			ThrowException(Exception::Error(String::New("We haven't implemented this array type yet!")));
		}
	}
	else {
		ThrowException(Exception::Error(String::New("We haven't implemented this type yet!")));
	}
	g_settings_sync();
	return scope.Close(Boolean::New(status));
}

void init(Handle<Object> target) {
	// in addon initialization function
	// data_symbol = NODE_PSYMBOL("data");
	// tag_symbol = NODE_PSYMBOL("tag");

	target->Set(String::NewSymbol("set_gsetting"),
	            FunctionTemplate::New(set_gsetting)->GetFunction());
	target->Set(String::NewSymbol("get_gsetting"),
	            FunctionTemplate::New(get_gsetting)->GetFunction());
	target->Set(String::NewSymbol("get_gsetting_keys"),
	            FunctionTemplate::New(get_gsetting_keys)->GetFunction());
}

NODE_MODULE(nodegsettings, init)