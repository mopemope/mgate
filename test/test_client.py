import memcache

client = memcache.Client(["localhost:5000"])

def test_set():
    for i in xrange(1):
        ret = client.set("A", "B")
        print "set result %s" % ret
        assert ret

def test_get():

    for i in xrange(1):
        ret = client.get("A")
        print "get result %s" % ret
        assert ret


