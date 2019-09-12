#ifndef RSLB_QUEUE_H
#define RSLB_QUEUE_H

struct queue_item {
    void* contents;
    struct queue_item* next;
};
struct queue_root {
    struct queue_item* head;
    struct queue_item* tail;
};

void init_queue(struct queue_root* queue);
void enqueue(struct queue_root* queue, struct queue_item* item);
struct queue_item* dequeue(struct queue_root* queue);

#endif
