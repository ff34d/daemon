#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "include/logger.h"
#include "include/proxy_worker.h"

int main() {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, proxy_worker, NULL) != 0) {
        log_message(LOG_ERROR, "MAIN", "Failed start proxy thread");
        return 1;
    }
    pthread_detach(thread_id);

    usleep(10000);

    while (1) {
        sleep(5);
    }

    return 0;
}
