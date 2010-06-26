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
setup_servers(MemClient *client, PyObject *list)
{
    PyObject *iterator;
    PyObject *item;
    char *host;
    iterator = PyObject_GetIter(list);
    if(PyErr_Occurred()){
        return -1;
    }
    while((item =  PyIter_Next(iterator))){
        if(PyString_Check(item)){
            host = PyString_AS_STRING(item);
            add_server(client->consistent, host);
        }else{
            PyErr_SetString(PyExc_ValueError,"server must string list");
            return -1;
        }
    }

    update_consistent(client->consistent);
    return 0;
}



PyObject *  
MemClient_New(ServerObject *server, PyObject *list)
{
    consistent_t *consistent;
    
    MemClient *self;

    self = PyObject_NEW(MemClient, &MemClientType);
    if(self == NULL){
        return NULL;
    }
    self->consistent = init_consistent();
    self->gate_server = server;
    
    return (PyObject *)self;
}

PyObject * 
MemClient_get(MemClient *self, PyObject *key)
{
    
    Py_RETURN_NONE;
}


static void
MemClient_dealloc(MemClient* self)
{
    PyObject_DEL(self);
}

static PyMethodDef Server_method[] = {
    { "get",      (PyCFunction)MemClient_get, METH_VARARGS, 0 },
    { NULL, NULL}
};

PyTypeObject MemClientType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "mgate.MemClient",             /*tp_name*/
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
