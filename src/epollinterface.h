#ifndef RSLB_EPOLLINTERFACE_H
#define RSLB_EPOLLINTERFACE_H

struct epoll_event_handler {
    int fd;
    /* A callback function to handle an epoll event. */
    void (*handle)(struct epoll_event_handler*, uint32_t);
    /*
     * A place to store any data the callback function
     * needs to do its job.
     */
    void* closure;
};

extern void epoll_init();

extern void epoll_add_handler(struct epoll_event_handler* handler, uint32_t event_mask);

extern void epoll_remove_handler(struct epoll_event_handler* handler);

extern void epoll_add_to_free_list(void* block);

extern void epoll_do_reactor_loop();

#endif