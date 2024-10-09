#include "curl_func.h"

int post_message(void *message)
{
	CURL *curl;
	// CURLcode res;

	curl = curl_easy_init();
	if (curl) {

		char *field_1;
		// char *field_2;
		notification_message_t *notification_message = NULL;
		notification_message = malloc(sizeof(notification_message_t));
		notification_message = (notification_message_t *)message;
		field_1 = strcat("title=", notification_message->title);
		printf("hello %s", field_1);
		/*	field_2 = strcat("message=", notification_message->message);
			curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/message?token=A35sWku4nvbqVdm");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strcat(field_1, field_2));

			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				curl_easy_cleanup(curl);
				curl_global_cleanup();
				free(notification_message);
				return -1;
			}
			free(notification_message);
		}
		*/
		free(notification_message);
	}
	return 0;
}
