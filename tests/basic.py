import zmq
import sys
import timeit
import json


if __name__ == "__main__":
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://192.168.214.29:5000")

    for x in xrange(0,100):
        msg = {'SET': "key_%d" % x,
               'value': "something%d" % x}
        socket.send(json.dumps(msg))
        print "Sending", msg
        msg_in = socket.recv()
        print(msg_in)

    for x in xrange(0,100):
        msg = {'GET': "key_%d" % x}
        socket.send(json.dumps(msg))
        print "Sending", msg
        msg_in = socket.recv()
        print(msg_in)

    for x in xrange(0,100, 2):
        keys = []
        keys.append("key_%d" % x)
        y = x + 1
        keys.append("key_%d" % y)
        msg = {'GET': keys}
        socket.send(json.dumps(msg))
        print "Sending", msg
        msg_in = socket.recv()
        print(msg_in)

    if len(sys.argv) > 1 and sys.argv[1] == "kill":
        msg = {'SHUTDOWN': True}
        socket.send(json.dumps(msg))
        print "Sending",msg
        msg_in = socket.recv()
        print(msg_in)
