#include "structs.h"
#define INITIAL_ARRAY_SIZE 1;

array_list_struct_t * create_array_list(){
    array_list_struct_t * array_list;
    array_list = malloc(sizeof(array_list_struct_t));  
    array_list->array_capacity = INITIAL_ARRAY_SIZE;
    array_list->array_size = 0; 
    array_list->data = malloc(sizeof(void *) * array_list->array_capacity);
    return array_list;
}

void add_array_element(array_list_struct_t *list, void *element){
 if(list && list->array_size >=list->array_capacity){
    int new_capacity = list->array_capacity * 2;
    list->data = realloc(list->data, sizeof(void *) * new_capacity);
    list->array_capacity = new_capacity;
 }
 list->data[list->array_size++] = element;
}
void free_array_list(array_list_struct_t *list){
    if(list){
        for(int i=0; i<list->array_size; i++){
            free(list->data[i]);
        }
        free(list->data);
        free(list);
    }
    else{
        return;
    }
}
