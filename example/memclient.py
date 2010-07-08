from mgate import *
print "start server"

class Simple(Server):
    
    cache = dict(A="A")
    lock = dict()
    cas_seq = dict()

    def get(self, env):
        c = self.MemClient(["localhost:11211"])
        print c
        c.get("A", self.on_get)

    def on_get(self):
        print self

        


if __name__ == '__main__':
    
    server = Simple()
    server.listen("0.0.0.0", 5000)
    server.run()


