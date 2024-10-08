#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <stdio.h>
#include "redis_lib.h"
#include "/home/osama/src/datastructs-c/arraylist.h"

arraylist *array = NULL;

int connect_redis(redisContext **c, const char *hostname, int port)
{
	*c = redisConnect(hostname, port);
	if ((*c)->err)
	{
		printf("Error: %s\n", (*c)->errstr);
		return -1;
	}
	printf("Connected to redis server\n");
	return 0;
}

void *get_redis_keys(redisContext *c)
{
	redisReply *reply = redisCommand(c, "KEYS *");
	if (reply->type == REDIS_REPLY_ARRAY)
	{
		arraylist *reply_array = arraylist_create();
		for (int i = 0; i < reply->elements; i++)
		{
			//	printf("%s\n", reply->element[i]->str);
			arraylist_add(reply_array, hi_strdup(reply->element[i]->str));
		}
		freeReplyObject(reply);
		return reply_array;
	}
	else if (reply->type == REDIS_REPLY_ERROR)
	{
		for (int i = 0; i < reply->elements; i++)
		{
			printf("%s\n", reply->element[i]->str);
		}
	}

	freeReplyObject(reply);
	return NULL;
}

void *get_redis_value(redisContext *c, const char *key)
{

	redisReply *reply = redisCommand(c, "GET %s", key);
	if (reply->type == REDIS_REPLY_STRING)
	{
		//		printf("%s\n", reply->str);
		
		char *str = hi_strdup(reply->str);

		freeReplyObject(reply);

		return str;
	}
	else if (reply->type == REDIS_REPLY_ERROR)
	{
		printf("%s\n", reply->str);
	}
	return NULL;
}

void* get_redis_json_value(redisContext *c, const char *key)
{
	redisReply *reply = redisCommand(c, "JSON.GET %s", key);
	if (reply->type == REDIS_REPLY_STRING)
	{
		//printf("%s\n", reply->str);
		char *str = hi_strdup(reply->str);
		freeReplyObject(reply);
		return str;
	}
	else if (reply->type == REDIS_REPLY_ERROR)
	{
		printf("Error occurred: %s\n", reply->str);
	}
	return NULL;
}

void* write_value_to_redis_json_array(redisContext *c, char *key,int array_index,char *path, char *value)
{
	char *command = (char *)malloc(100);
	sprintf(command,"JSON.SET %s $[%d].%s %s", key,array_index,path, value);
	printf("%s\n",command);
	redisReply *reply = redisCommand(c, command);
	if (reply->type == REDIS_REPLY_STRING)
	{
		freeReplyObject(reply);
		free(command);
		return 0;
	}
	else if (reply->type == REDIS_REPLY_ERROR)
	{
		printf("Error occurred in JSON value: %s\n", reply->str);
	}
	free(command);
	freeReplyObject(reply);
	return NULL;
}
