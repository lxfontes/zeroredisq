#include <zmq.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "zrq_zmq.h"


zrq_zmq_t *zrq_zmq_create(const char *endpoint, zrq_zmq_cb cb)
{
	zrq_zmq_t *ret;

	ret = malloc(sizeof(*ret));
	if (ret == NULL) return NULL;

	ret->context = zmq_init(1);

	ret->endpoint = strdup(endpoint);
	ret->cb = cb;

	ret->responder = zmq_socket (ret->context, ZMQ_REP);
	zmq_bind (ret->responder, ret->endpoint);
	return ret;
}

void zrq_zmq_destroy(zrq_zmq_t *z)
{
	zmq_term(z->context);
	free(z);
}

void zrq_zmq_stop(zrq_zmq_t *z)
{
	z->running = 0;
}

void _zrq_free(void *data, void *hint)
{
	free(data);
}

void zrq_zmq_run(zrq_zmq_t *z)
{
	z->running = 1;
	char *data_in;
	size_t len_in;
	char *data_out;
	size_t len_out;
	zmq_msg_t request;
	zmq_msg_t reply;

	while (z->running) {
		zmq_msg_init (&request);
		zmq_recv (z->responder, &request, 0);
		len_in = zmq_msg_size(&request);
		data_in = zmq_msg_data(&request);

		len_out = 0;

		if (z->cb(z, data_in, len_in, &data_out, &len_out)) {
			//callback didn't provide data, send blank
			data_out = strdup("\n");
			len_out = 1;
		}

		zmq_msg_close (&request);

		zmq_msg_init_data (&reply, data_out, len_out, _zrq_free, NULL);
		zmq_send (z->responder, &reply, 0);
		zmq_msg_close (&reply);
	}
}
