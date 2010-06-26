#include "mc_consistent.h"

static void 
parse_host(memserver_t server, char *server_string){
    int port = 11211;
    char *ptr;
    
    char buffer[256];

    size_t length= strlen(server_string);
    memcpy(buffer, server_string, length);
    buffer[length]= 0;

    ptr = index(buffer, ':');

    if (ptr)
    {
      ptr[0]= 0;

      ptr++;

      port= strtoul(ptr, (char **)NULL, 10);
    }
    server.hostname = buffer;
    server.port = port;
}

consistent_t * 
init_consistent(void)
{
    consistent_t *consistent;
    consistent = malloc(sizeof(consistent_t));
    
    /* MAX 10 server */
    consistent->server_list = malloc(sizeof(memserver_t) * MAX_SERVER);
    consistent->server_count = 0;
    return consistent;
}

void
add_server(consistent_t *consistent, char *host)
{

    uint32_t index = 0;
    index = consistent->server_count;
    if(index > MAX_SERVER){
        return;
    }
    
    parse_host(consistent->server_list[index], host);

    consistent->server_count++;
    consistent->points_count = POINTS_PER_SERVER * consistent->server_count;
}


static uint32_t 
hash_one_at_a_time(const char *key, size_t key_length);

static int 
point_item_cmp(const void *t1, const void *t2)
{
    point_item_t *ct1= (point_item_t *)t1;
    point_item_t *ct2= (point_item_t *)t2;

    if (ct1->value == ct2->value){
        return 0;
    }else if (ct1->value > ct2->value){
        return 1;
    }else{
        return -1;
    }
}

void
update_consistent(consistent_t *consistent)
{
    uint32_t server_count;

    uint32_t point_index = 0;
    uint32_t server_index = 0;
    uint32_t i = 0;
    uint32_t counter= 0;
    uint32_t per_server= POINTS_PER_SERVER;
    uint32_t value;

    memserver_t *list = consistent->server_list;
    server_count = consistent->server_count;
    if(!consistent->points){ 
        consistent->points = malloc(sizeof(point_item_t) * POINTS_PER_SERVER * server_count);
    }

    
    for (server_index= 0; server_index < server_count; ++server_index) {

        for (i = 1; i <= per_server; i++){

            char host[MAX_HOST_LENGTH]= "";
            size_t host_length;
            
            host_length = (size_t) snprintf(host, MAX_HOST_LENGTH,
                                                  "%s:%u-%u",
                                                  list[server_index].hostname,
                                                  list[server_index].port,
                                                  i -1);
            value = hash_one_at_a_time(host, host_length);
            
            //printf("%s point %u\n", host, value);
            consistent->points[point_index].index = server_index;
            consistent->points[point_index].value = value;
            point_index++;
        }

    }
    
    //sort
    qsort(consistent->points, point_index, sizeof(point_item_t), point_item_cmp);
}

static memserver_t
get_server(consistent_t *consistent, char *key, size_t key_length)
{
    uint32_t hash, index;
    hash = hash_one_at_a_time(key, key_length);
    uint32_t length = consistent->points_count;

    hash = hash;
    point_item_t *begin, *end, *left, *right, *middle;
    begin = left= consistent->points;
    end = right= consistent->points + length;

    while (left < right){
        middle = left + (right - left) / 2;
        if (middle->value < hash){
            left= middle + 1;
        }else{
            right= middle;
        }
    }
    if (right == end){
        right = begin;
    }
    return consistent->server_list[right->index];
}


static uint32_t 
hash_one_at_a_time(const char *key, size_t key_length)
{
    const char *ptr= key;
    uint32_t value= 0;

    while (key_length--)
    {
        uint32_t val= (uint32_t) *ptr++;
        value += val;
        value += (value << 10);
        value ^= (value >> 6);
    }
    value += (value << 3);
    value ^= (value >> 11);
    value += (value << 15);

    return value;
}


