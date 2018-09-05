#include "txinm.h"

#include <stdio.h>

typedef struct txim_test_entity txim_test_entity;
struct txim_test_entity {
    int id;
    txim_tx_value* val;
};

txim_test_entity* create_int_entity(int t_id) {
    txim_test_entity* entity = malloc(sizeof(txim_test_entity));
    txim_raw_value* raw_val = txim_create_raw_value(&txim_destory_int_data, &txim_copy_int_data);
    txim_tx_value* tx_val = txim_create_tx_value(raw_val);
    tx_val->t_id = t_id;
    entity->id = 1;
    entity->val = tx_val;
    return entity;
}

void destory_int_entity(txim_test_entity* entity) {
    if (entity == NULL) {
        return;
    }

    txim_destory_tx_value(entity->val);
    entity->val = NULL;
    free(entity);
}


int main()
{
    int t_id = 1;
    int int_val = 1000;

    txim_test_entity* entity = create_int_entity(t_id);
    t_id+=1;

    //![start transaction No.1]
    int lock_reuslt = tx_value_try_lock_for_update(entity->val, t_id);
    if (lock_reuslt < 0) {
        printf("lock error lock_reuslt: %d", lock_reuslt);
        return lock_reuslt;
    }

    tx_value_set_value(entity->val, t_id, &int_val);
    tx_value_commit(entity->val, t_id);
    t_id += 1;

    int* commit_val =(int*) tx_value_get_value(entity->val, t_id);

    printf("commit_val %d\n", *commit_val);
    //![end transaction No.1]

    //![start transaction No.2]
    int_val = 1001;
    lock_reuslt = tx_value_try_lock_for_update(entity->val, t_id);
    if (lock_reuslt < 0) {
        printf("lock error lock_reuslt: %d", lock_reuslt);
        return lock_reuslt;
    }

    tx_value_set_value(entity->val, t_id, &int_val);

    int* un_commit_val =(int*) tx_value_get_value(entity->val, t_id);

    printf("un_commit_val %d\n", *un_commit_val);

    tx_value_rollback(entity->val, t_id);
    t_id += 1;

    commit_val =(int*) tx_value_get_value(entity->val, t_id);
    printf("final val %d\n", *commit_val);
    //![end transaction No.2]

    destory_int_entity(entity);

    return 0;
}

