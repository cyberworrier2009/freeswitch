#include <hiredis/hiredis.h>

typedef struct {
	char *robocall_id;
	char **call_number;
	char *call_status;
}call_info;

typedef call_info *call_info_t;
int connect_redis(redisContext **c,const char *hostname, int port);
void* get_redis_keys(redisContext *c);
void* get_redis_value(redisContext *c, const char *key);
void* get_redis_json_value(redisContext *c, const char *key);
void* write_value_to_redis_json_array(redisContext *c,char *key,int array_index,char *path, char *value);
