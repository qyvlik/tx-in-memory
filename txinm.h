#ifndef TRANSACTION_VALUE_HANDLE
#define TRANSACTION_VALUE_HANDLE

#include <pthread.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct txim_raw_value txim_raw_value;
struct txim_raw_value  {
    void* data;
    void* un_commit_data;
    void (* destory_data)(void *);
    void* (* copy_data)( void *);
};

typedef void (* txim_destory_data_fun)(void *);
typedef void* ( txim_copy_data_fun)(void *);

void txim_destory_int_data(void *data);
void* txim_copy_int_data(void* data);

void txim_raw_value_set_un_commit_data(txim_raw_value* val, void *data);
void* txim_raw_value_get_data(txim_raw_value* val);
void* txim_raw_value_get_un_commit_data(txim_raw_value* val);
void txim_raw_value_commit(txim_raw_value* val);
void txim_raw_value_rollback(txim_raw_value* val);
txim_raw_value* txim_create_raw_value(txim_destory_data_fun destory_data, txim_copy_data_fun copy_data);
void txim_destory_raw_value(struct txim_raw_value* val);

typedef struct txim_atomic_callback txim_atomic_callback;
struct txim_atomic_callback {
    int (*callback)(txim_raw_value*, int, void** args);
};

typedef struct txim_tx_value txim_tx_value;
struct txim_tx_value {
    txim_raw_value* raw_value;
    volatile int t_id;
    pthread_mutex_t mutex;
};

txim_tx_value* txim_create_tx_value(txim_raw_value* raw_value);
void txim_destory_tx_value(txim_tx_value* tx_val);

const void* tx_value_get_readonly_value(txim_tx_value* tx_val);
const void* tx_value_get_value(txim_tx_value* tx_val, int t_id);
int tx_value_lock_for_update(txim_tx_value* tx_val, int t_id);
int tx_value_try_lock_for_update(txim_tx_value* tx_val, int t_id);
int tx_value_set_value(txim_tx_value* tx_val, int t_id, void* val);
int tx_value_commit(txim_tx_value* tx_val, int t_id);
int tx_value_rollback(txim_tx_value* tx_val, int t_id);

#ifdef __cplusplus
}                   //  extern "C" {
#endif

#endif // TRANSACTION_VALUE_HANDLE
