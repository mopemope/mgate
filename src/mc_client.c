#include "mc_client.h"

#define BUFSIZE 4096


static void
server_init(memserver_t *server)
{
    PyObject *env;
    server->input_buf = malloc(sizeof(char) * BUFSIZE);
    server->input_buf_size = sizeof(char) * BUFSIZE;
    server->input_pos = 0;
    server->input_len = 0;
    server->parser = (mctext_parser *)PyMem_Malloc(sizeof(mctext_parser));
    env = PyDict_New();

    mctext_init(server->parser, env);
}

static void
server_t_free_field(memserver_t *server)
{

    
}

static int
_connect(memserver_t *server)
{
    return 0;
}




static void
MemClient_dealloc(MemClient* self)
{
    PyObject_DEL(self);
}


PyTypeObject MemClientType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "memproto.MemcClient",             /*tp_name*/
    sizeof(MemClient), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)MemClient_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0, /*ResponseObject_str*/                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "memcached client ",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                      /* tp_init */
    0,                         /* tp_alloc */
    0,                           /* tp_new */
};
