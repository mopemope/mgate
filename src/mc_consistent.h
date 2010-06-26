#include "mc_client.h"

consistent_t * 
init_consistent(void);

void
add_server(consistent_t *consistent, char *host);

void
update_consistent(consistent_t *consistent);

