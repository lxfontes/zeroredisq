# Overview

memory based key=value store over json as protocol.

ZeroMQ binds to port 5000 as responder (ZMQ_REP).

Clients connect using ZMQ_REQ. See tests/basic.py for example.

# Operations

## GET

{ 'GET': 'key1' }

and

{ 'GET': ['key1','key2','key3'] }

## SET

{ 'SET': 'key1',
  'value': 'something'
}

# Performance

Obviously this is *not* close to redis or memcache.

It averages 12k operations using a single ZeroMQ client on a macbook pro i7 8gb.
