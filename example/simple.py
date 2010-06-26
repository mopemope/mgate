from mgate import *
print "start server"

class Simple(Server):
    
    cache = dict(A="A")
    lock = dict()
    cas_seq = dict()

    def get(self, env):
        #print env
        k = env['key']
        #return self.cache[k]
        #print "get:" + k
        #print "get: %s" % k
        #return self.cache[k]
        #import time 
        #time.sleep(10)
        data = self.cache.get(k, ("",0))
        #print data
        self.write(env, data)
        #print "get %d" % len(v)
        #return v

    def set(self, env):
        #print env
        k = env['key']
        v = env['data']
        flags = env["flags"]
        #self.cache[k] = v
        self.cache[k] = (v, flags)
        if not self.cas_seq.has_key(k):
            self.cas_seq[k] = 1
        #print "set: %s" % k
        #print self.cache
        self.write(env, True)

    def delete(self, env):
        print env
        k = env['key']
        if self.cache.has_key(k):
            del self.cache[k]
        self.write(env, True)

    def incr(self, env):
        print env
        v = env['value']
        self.write(env, v+1)

    def decr(self, env):
        v = env['value']
        self.write(env, v-1)
    
    def gets(self, env):
        #print env
        k = env['key']
        ret = self.cache.get(k, None)
        if not ret:
            ret = ("", 0, 1)
        else:
            cas = self.lock.get(k, None)
            if not cas:
                cas = self.cas_seq[k] + 1
                self.cas_seq[k] = cas
                self.lock[k] = cas

            ret = ret + (cas,)
        #print ret
        self.write(env, ret)

    def cas(self, env):
        #print env
        k = env['key']
        v = env['data']
        flags = env["flags"]
        cas = env["cas_unique"]
        
        lock = self.lock.get(k, None)
        if lock:
            if cas == lock:
                #print "cas ok"
                self.cache[k] = (v, flags)
                del self.lock[k]
            else:
                return self.write(env, False)
        else:
            self.cache[k] = (v, flags)
            if not self.cas_seq.has_key(k):
                self.cas_seq[k] = 1
        self.write(env, True)



        


if __name__ == '__main__':
    
    server = Simple()
    server.listen("0.0.0.0", 5000)
    server.run()


