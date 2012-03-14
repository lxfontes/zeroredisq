#include "zrq_zmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <db.h>


typedef struct zrq_db {
	DB *dbp;
} zrq_db_t;


int zrq_set_key(DB *dbp, const char *skey, size_t slen, const char *sval, size_t vlen)
{
	DBT key, val;
	int ret;

	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));

	key.data = (void *)skey;
	key.size = slen + 1;

	val.data = (void *)sval;
	val.size = vlen + 1;

	ret = dbp->put(dbp, NULL, &key, &val, 0);

	return ret;
}

int zrq_get_key(DB *dbp, const char *str, size_t len, DBT *retdbt)
{
	DBT key,val;
	int ret;

	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));

	key.data = (void *)str;
	key.size = len + 1;

	ret = dbp->get(dbp, NULL, &key, &val, 0);

	if (ret == DB_NOTFOUND) {
		return -1;
	}

	memcpy(retdbt, &val, sizeof(DBT));
	return 0;
}


int zrq_set(zrq_zmq_t *z, json_t *jreq, json_t *jresp)
{
	json_t *jop;
	json_t *jv;
	const char *key;
	zrq_db_t *db = ZRQ_ZMQ_DATA(z);
	int ret;
	char *sval;
	int vlen;

	jop = json_object_get(jreq, "SET");
	key = json_string_value(jop);
	jv = json_object_get(jreq, "value");

	if (jv == NULL) return 1;

	sval = json_dumps(jv, JSON_COMPACT | JSON_ENCODE_ANY);
	vlen = strlen(sval);

	ret = zrq_set_key(db->dbp, key, strlen(key), sval, vlen);
	free(sval);

	if (ret != 0) {
		json_object_set_new(jresp, "result", json_string("error"));
	} else {
		json_object_set_new(jresp, "result", json_string("ok"));
	}

	return 0;
}

int zrq_get(zrq_zmq_t *z, json_t *jreq, json_t *jresp)
{
	json_t *jop;
	json_t *el;
	json_error_t err;
	const char *key;
	int ret;
	zrq_db_t *db = ZRQ_ZMQ_DATA(z);
	DBT val;
	jop = json_object_get(jreq, "GET");

	if (json_is_array(jop)) {
		// 1 entry per key
		size_t nel = json_array_size(jop);
		size_t i = 0;
		for(;i < nel; i++) {
			el = json_array_get(jop, i);
			key = json_string_value(el);


			ret = zrq_get_key(db->dbp, key, strlen(key), &val);

			if (ret) {
				//doesnt exist
				json_object_set_new(jresp, key, json_null());
			}else{
				char *v = val.data;
				json_t *tj = json_loads(v, JSON_DECODE_ANY, &err);
				json_object_set(jresp, key, tj);
				json_decref(tj);
			}
		}
	} else if(json_is_string(jop)) {
		key = json_string_value(jop);

		ret = zrq_get_key(db->dbp, key, strlen(key), &val);

		if (ret) {
			//doesnt exist
			json_object_set_new(jresp, key, json_null());
		}else{
			char *v = val.data;
			json_t *tj = json_loads(v, JSON_DECODE_ANY, &err);
			json_object_set(jresp, key, tj);
			json_decref(tj);
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
	zrq_db_t db;
	u_int32_t flags;
	char *default_endpoint = "tcp://*:5000";
	char *endpoint;

	if (argc == 2)
		endpoint = argv[1];
	else
		endpoint = default_endpoint;

	db_create(&db.dbp, NULL, 0);

	flags = DB_CREATE | DB_PRIVATE;
	db.dbp->open(db.dbp, NULL, NULL, NULL, DB_BTREE, flags, 0);

	zmq = zrq_zmq_create(endpoint, zrq_callback);

	ZRQ_ZMQ_DATA(zmq) = &db;

	zrq_zmq_run(zmq);
	return 0;
}
