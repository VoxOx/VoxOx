
#include <wtimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//in sec
#define TEST_TIME 10

//in msec
#define DELAY 100

struct timer_impl *timpl;

void callback(void *userdata) {
    int *data = (int *) userdata;
    printf("Timer elapsed, data %d\n", *data);
}

w_timer_t * create_timer(int udata) {
    w_timer_t *ti;

    int *data = (int *) malloc (sizeof(int));
    *data = udata;

    ti = timpl->timer_create();
    timpl->timer_set_delay(ti, DELAY);
    timpl->timer_set_callback(ti, callback);
    timpl->timer_set_userdata(ti, data);
    timpl->timer_start(ti);

    return ti;
}

int main(int argc, char **argv) {
    w_timer_t *ti1, *ti2;

    timer_init();

    timpl = timer_impl_getfirst();

    ti1 = create_timer(1);
    ti2 = create_timer(2);

    sleep (TEST_TIME);

    timpl->timer_stop(ti1);
    timpl->timer_stop(ti2);
    return 0;
}
