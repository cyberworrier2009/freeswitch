#include "/home/osama/src/datastructs-c/arraylist.h"
#include "call_handling.h"
#include "redis_library/redis_lib.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

arraylist *call_schedule = NULL;
arraylist *call_process = NULL;
arraylist *call_pause_list = NULL;

void call_queue(char **numbers, int count)
{
	for (int i = 0; i < count; i++) { printf("%s,", numbers[i]); }
}

void schedule_calls()
{
	redisContext *c = NULL;
	int is_connected = connect_redis(&c, "localhost", 6379);

	if (is_connected == 0) {
		char *json_data = get_redis_json_value(c, "callup_queue");
		cJSON *root = cJSON_Parse(json_data);
		if (cJSON_IsArray(root)) {
			int array_size = cJSON_GetArraySize(root);

			if (array_size == 0) {
				printf("No data in the queue\n");
				return;
			} else {
				call_info_t call = (call_info *)malloc(sizeof(call_info));
				call_schedule = arraylist_create();
				for (int i = 0; i < array_size; i++) {
					cJSON *array_json_item = cJSON_GetArrayItem(root, i);
					if (strcmp(cJSON_GetObjectItem(array_json_item, "call_status")->valuestring, "Initiated") == 0) {
						cJSON *call_numbers = cJSON_GetObjectItem(array_json_item, "numbers");
						call->call_number = (char **)malloc(sizeof(char *) * cJSON_GetArraySize(call_numbers));
						if (cJSON_IsArray(call_numbers)) {
							call->call_number = (char **)malloc(sizeof(char *) * cJSON_GetArraySize(call_numbers));
							for (int j = 0; j < cJSON_GetArraySize(call_numbers); j++) {
								cJSON *number = cJSON_GetArrayItem(call_numbers, j);
								call->call_number[j] = (char *)number->valuestring;
								strcpy(call->call_number[j], number->valuestring);
							}
						}
					}

					call->call_status = "In-Progress";
					call->robocall_id = cJSON_GetObjectItem(array_json_item, "request_identifier")->valuestring;
					arraylist_add(call_schedule, call);
					char *json = "{\"call_status\":\"In-Progress\"}";
					write_value_to_redis_json_array(c,"callup_queue",i,"call_status",json);
				}
			}
		}
	} else {
		printf("Connection failed\n");
	}
}

int main()
{
	schedule_calls();
	// switch_load();
	/*char* numbers[] = {"1234","5678","9101"};*/
	/*int n = sizeof(numbers)/sizeof(numbers[0]);*/
	/*call_queue(numbers,n);*/
	return 0;
}
