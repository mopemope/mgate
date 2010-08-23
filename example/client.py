import memcache 

def test():
    client = memcache.Client(["localhost:11211"])
    client.set("A", "B")
    ret = client.get("A")
    assert ret == "B"

def test_py():
    client = memcache.Client(["localhost:5000"])
    client.set("A", "B")
    ret = client.get("A")
    assert ret == "B"

def test_cas():
    client = memcache.Client(["localhost:5000"])
    client.set("A", "B")
    ret = client.gets("A")
    assert ret == "B"
    #print client.cas_ids
    client.cas("A", "C")
    ret = client.gets("A")
    assert ret == "C"



if __name__ == '__main__':

    from timeit import Timer
    
    t = Timer("test()", "from __main__ import test")
    print t.repeat(number=1000)
    
    t = Timer("test_py()", "from __main__ import test_py")
    print t.repeat(number=1000)
    
    #t = Timer("test_cas()", "from __main__ import test_cas")
    #print t.repeat(number=1000)
    #client = memcache.Client(["localhost:5000"])
    #client.set("A")




