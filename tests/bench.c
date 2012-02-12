#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

void fake_free(void *a,void *b){}

static long long mstime(void) {
	struct timeval tv;
	long long mst;

	gettimeofday(&tv, NULL);
	mst = ((long)tv.tv_sec)*1000;
	mst += tv.tv_usec/1000;
	return mst;
}
int main (int argc, char **argv)
{
	void *context = zmq_init (1);
	int i;
	int total_reqs = atoi(argv[1]);
	void *responder = zmq_socket (context, ZMQ_REQ);
	long long start;
	long long stop;
	zmq_connect(responder, "tcp://127.0.0.1:5000");

	char *reqs = "{\"SET\": \"key\",\"value\": 100}";
	char *reqg = "{\"GET\": \"key\"}";
	int sizes = strlen(reqs);
	int sizeg = strlen(reqg);


	zmq_msg_t request;
	zmq_msg_t reply;

	start = mstime();
	for(i=0;i<total_reqs;i++) {
		zmq_msg_init_data (&request, reqs, sizes, fake_free,NULL);
		zmq_send (responder, &request, 0);
		zmq_msg_close (&request);

		zmq_msg_init(&reply);
		zmq_recv(responder, &reply, 0);
		zmq_msg_close (&reply);
	}
	stop = mstime();
	printf("SET: Elapsed %lld ms, req/s %lld\n", (stop-start), (total_reqs / (stop-start)) * 1000);

	start = mstime();
	for(i=0;i<total_reqs;i++) {
		zmq_msg_init_data (&request, reqg, sizeg, fake_free,NULL);
		zmq_send (responder, &request, 0);
		zmq_msg_close (&request);

		zmq_msg_init(&reply);
		zmq_recv(responder, &reply, 0);
		zmq_msg_close (&reply);
	}
	stop = mstime();
	printf("GET: Elapsed %lld ms, req/s %lld\n", (stop-start), (total_reqs / (stop-start)) * 1000);
	zmq_close (responder);
	zmq_term (context);
	return 0;
}
