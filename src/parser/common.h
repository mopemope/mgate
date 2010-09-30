#ifndef PARSER_H
#define PARSER_H

#include "../client.h"

/* env key */
extern PyObject *cmd_key;
extern PyObject *client_key;
extern PyObject *key_key;
extern PyObject *value_key;
extern PyObject *flags_key;
extern PyObject *data_key;
extern PyObject *exptime_key;
extern PyObject *noreply_key;
extern PyObject *cas_key;

/* method_name */

extern PyObject *m_get;
extern PyObject *m_gets;
extern PyObject *m_set;
extern PyObject *m_append;
extern PyObject *m_add;
extern PyObject *m_replace;
extern PyObject *m_prepend;
extern PyObject *m_delete;
extern PyObject *m_incr;
extern PyObject *m_decr;
extern PyObject *m_cas;

inline void
init_parser_key(void);

inline void
fin_parser_key(void);







#endif

