from setuptools import setup, find_packages, Extension
import os
import sys
import os.path

readme = ""

if "posix" not in os.name:
    print "Are you really running a posix compliant OS ?"
    print "Be posix compliant is mandatory"
    sys.exit(1)

library_dirs=['/usr/local/lib']
include_dirs=[]



setup(name='mgate',
        version="0.1",
        description="",
        long_description=readme,
        keywords='',
        author='yutaka matsubara',
        author_email='yutaka.matsubara@gmail.com',
        test_suite = 'nose.collector',        
        packages= find_packages(),
        ext_modules = [
            Extension('mgate',
            sources=['src/server.c', 'src/picoev_epoll.c', 
                #'src/parser/common.c', 
                'src/parser/text_parser.c', 
                'src/parser/binary_parser.c',
                'src/client.c', 'src/memproto/memtext.c','src/memproto/memproto.c',
                'src/response/response.c',
                'src/response/text_response.c',
                'src/response/binary_response.c',
                'src/util.c', ],
            #'src/mc_client.c', 'src/mc_consistent.c', 'src/mc_net.c'],
            include_dirs=include_dirs,
            library_dirs=library_dirs,
            #libraries=["profiler"],
            extra_compile_args=["-DDEBUG"],
            )
        ]
)


