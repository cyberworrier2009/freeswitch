#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

typedef struct pipe_message {
	char *title;
	char *message;
} notification_message_t;

int post_message(void *message);

