#include "queue.h"

void init_queue(struct queue_root* queue){
    queue->head = queue->tail = 0;
}

void enqueue(struct queue_root* queue, struct queue_item* item) {
    item->next = 0;
    if (queue->head == 0){
        queue->head = queue->tail = item;
    } else {
        queue->tail = queue->tail->next = item;
    }
}

struct queue_item* dequeue(struct queue_root* queue) {
    struct queue_item* item;
    if (queue->head == 0){
        return 0;
    } else {
        item = queue->head;
        struct queue_item* next = queue->head->next;
        queue->head = next;
        if (queue->head == 0)
            queue->tail = 0;
    }
    return item;
}
