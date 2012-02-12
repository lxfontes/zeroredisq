#ifndef __ZRQ_ZMQ_H__
#define __ZRQ_ZMQ_H__

#include <string.h>

struct zrq_zmq;
typedef int (*zrq_zmq_cb)(struct zrq_zmq *z, char *data_in,
			   size_t len_in, char **data_out,
			   size_t *len_out);

typedef struct zrq_zmq {
	char *endpoint;
	void *data;
	zrq_zmq_cb cb;

	void *context;
	void *responder;
	int running;
} zrq_zmq_t;

#define ZRQ_ZMQ_DATA(z) z->data

zrq_zmq_t *zrq_zmq_create(const char *endpoint, zrq_zmq_cb cb);
void zrq_zmq_run(zrq_zmq_t *z);
void zrq_zmq_stop(zrq_zmq_t *z);
void zrq_zmq_destroy(zrq_zmq_t *z);
#endif
