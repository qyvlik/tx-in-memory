#include "txinm.h"

void txim_destory_int_data(void *data) {
    if (data) {
        int* d = (int*)data;
        free(d);
    }
}

void* txim_copy_int_data(void* data) {
    int* d = (int*) data;
    int* new_data = (int *) malloc(sizeof(int));
    *new_data = *d;
    return new_data;
}

void txim_raw_value_set_un_commit_data(txim_raw_value* val, void *data) {
    void* new_data = val->copy_data(data);
    if (val->un_commit_data) {
        val->destory_data(val->un_commit_data);
    }
    val->un_commit_data = new_data;
}

void* txim_raw_value_get_data(txim_raw_value* val) {
    return val->data;
}

void* txim_raw_value_get_un_commit_data(txim_raw_value* val) {
    return val->un_commit_data;
}

void txim_raw_value_commit(txim_raw_value* val) {
    void* old_data = val->data;
    val->data = val->un_commit_data;
    val->un_commit_data = NULL;
    val->destory_data(old_data);
}

void txim_raw_value_rollback(txim_raw_value* val) {
    val->destory_data(val->un_commit_data);
    val->un_commit_data = NULL;
}

txim_raw_value* txim_create_raw_value(txim_destory_data_fun destory_data, txim_copy_data_fun copy_data) {
    txim_raw_value* val = malloc(sizeof(txim_raw_value));

    val->data = NULL;
    val->un_commit_data = NULL;
    val->destory_data = destory_data;
    val->copy_data = copy_data;

    return val;
}

void txim_destory_raw_value(txim_raw_value* val) {
    if (val == NULL) {
        return;
    }
    val->destory_data(val->data);
    val->data = NULL;
    val->destory_data(val->un_commit_data);
    val->un_commit_data = NULL;
    free(val);
}

//--

const void* tx_value_get_readonly_value(txim_tx_value* tx_val) {
    return tx_val->raw_value->data;
}

const void* tx_value_get_value(txim_tx_value* tx_val, int t_id) {
    if (t_id == 0 || tx_val->t_id != t_id) {
        return tx_val->raw_value->data;
    }
    return tx_val->raw_value->un_commit_data;
}

int tx_value_lock_for_update(txim_tx_value* tx_val, int t_id) {
    int result = pthread_mutex_lock(&tx_val->mutex);
    if (result != 0) {
        return -result;
    }
    tx_val->t_id = t_id;

    txim_raw_value_set_un_commit_data(tx_val->raw_value,
                                      tx_val->raw_value->copy_data(tx_val->raw_value->data));

    return 1;
}

int tx_value_try_lock_for_update(txim_tx_value* tx_val, int t_id) {
    int result = pthread_mutex_trylock(&tx_val->mutex);
    if (result) {
        return -1;
    }
    tx_val->t_id = t_id;
    return 1;
}

int tx_value_set_value(txim_tx_value* tx_val, int t_id, void* val) {
    if (t_id == 0 || t_id != tx_val->t_id) {
        return -1;
    }

    txim_raw_value_set_un_commit_data(tx_val->raw_value, val);

    return 1;
}

int tx_value_commit(txim_tx_value* tx_val, int t_id) {
    if (t_id == 0 || t_id != tx_val->t_id) {
        return -1;
    }

    txim_raw_value_commit(tx_val->raw_value);
    pthread_mutex_unlock(&tx_val->mutex);
    tx_val->t_id = 0;

    return 1;
}

int tx_value_rollback(txim_tx_value* tx_val, int t_id) {
    if (t_id == 0 || t_id != tx_val->t_id) {
        return -1;
    }
    txim_raw_value_rollback(tx_val->raw_value);
    pthread_mutex_unlock(&tx_val->mutex);
    tx_val->t_id = 0;
    return 1;
}

txim_tx_value* txim_create_tx_value(txim_raw_value *raw_value) {
    txim_tx_value* tx_val = malloc(sizeof(txim_tx_value));
    tx_val->t_id = 0;
    tx_val->raw_value = raw_value;
    pthread_mutex_init(& tx_val->mutex, NULL);

    return tx_val;
}

void txim_destory_tx_value(txim_tx_value *tx_val)
{
    pthread_mutex_destroy(&tx_val->mutex);
    tx_val->t_id = 0;
    if (tx_val->raw_value) {
        txim_destory_raw_value(tx_val->raw_value);
        tx_val->raw_value = NULL;
    }
    free(tx_val);
}
