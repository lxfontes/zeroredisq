#include "zrq_zmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include "uthash.h"
#include <jemalloc/jemalloc.h>

typedef struct db_entry {
	char *key;
	json_t *val;
	UT_hash_handle hh;
} db_entry_t;


int zrq_set(zrq_zmq_t *z, json_t *jreq, json_t *jresp)
{
	json_t *jop;
	json_t *jv;
	json_t *value;
	const char *key;
	db_entry_t *db = ZRQ_ZMQ_DATA(z);
	db_entry_t *entry = NULL;

	jop = json_object_get(jreq, "SET");
	key = json_string_value(jop);
	jv = json_object_get(jreq, "value");

	if (jv == NULL) return 1;

	value = json_copy(jv);

	HASH_FIND_STR(db, key, entry);

	if (entry) {
		json_decref(entry->val);
		entry->val = value;
	} else {
		entry = malloc(sizeof(*entry));
		if (entry == NULL) {
			json_decref(entry->val);
			json_object_set_new(jresp, "result", json_string("error"));
			return 0;
		}
		entry->key = strdup(key);
		entry->val = value;
		HASH_ADD_KEYPTR(hh, db, entry->key, strlen(entry->key), entry);
		ZRQ_ZMQ_DATA(z) = db;
	}

	json_object_set_new(jresp, "result", json_string("ok"));

	return 0;
}

int zrq_get(zrq_zmq_t *z, json_t *jreq, json_t *jresp)
{
	json_t *jop;
	json_t *el;
	const char *key;
	db_entry_t *db = ZRQ_ZMQ_DATA(z);
	db_entry_t *entry = NULL;
	jop = json_object_get(jreq, "GET");

	if (json_is_array(jop)) {
		// 1 entry per key
		size_t nel = json_array_size(jop);
		size_t i = 0;
		for(;i < nel; i++) {
			el = json_array_get(jop, i);
			key = json_string_value(el);


			HASH_FIND_STR(db, key, entry);

			if (entry) {
				json_object_set(jresp, key, entry->val);
			}else{
				json_object_set_new(jresp, key, json_null());
			}
		}
	} else if(json_is_string(jop)) {
		key = json_string_value(jop);
		HASH_FIND_STR(db, key, entry);
		if (entry) {
			json_object_set(jresp, key, entry->val);
		}else{
			json_object_set_new(jresp, key, json_null());
		}
	} else {
		return 1;
	}

	return 0;
}

int zrq_callback(zrq_zmq_t *z, char *data_in, size_t len_in, char **data_out, size_t *len_out)
{
	json_t *jreq;
	json_t *jresp;
	json_t *jop;
	json_error_t error;
	char *data;
	int r;


	jresp = json_object();
	jreq = json_loadb(data_in, len_in, 0, &error);

	jop = json_object_get(jreq, "GET");
	if (jop != NULL) {
		r = zrq_get(z, jreq, jresp);
	}else{
		jop = json_object_get(jreq, "SET");
		r = zrq_set(z, jreq, jresp);
	}

	data = json_dumps(jresp,JSON_COMPACT);

	json_decref(jreq);
	json_decref(jresp);

	*data_out = data;
	*len_out = strlen(data);
	return 0;
}

int main(int argc, char **argv)
{
	zrq_zmq_t *zmq;
	char *default_endpoint = "tcp://*:5000";
	char *endpoint;

	if (argc == 2)
		endpoint = argv[1];
	else
		endpoint = default_endpoint;

	zmq = zrq_zmq_create(endpoint, zrq_callback);

	ZRQ_ZMQ_DATA(zmq) = NULL;

	zrq_zmq_run(zmq);
	return 0;
}
