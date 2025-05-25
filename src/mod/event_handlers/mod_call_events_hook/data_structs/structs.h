#include<stdlib.h>
typedef struct array_list_struct {
    void **data;
    int array_size;
    int array_capacity;
} array_list_struct_t;

array_list_struct_t * create_array_list();
void add_array_element(array_list_struct_t *list, void *element);
void pop_array_element(array_list_struct_t *list);
static inline int array_size(array_list_struct_t *list){
    return list->array_size;
}
void free_array_list(array_list_struct_t *list);