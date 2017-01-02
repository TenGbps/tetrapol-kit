#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

// include, we are testing static methods
#include "tp_timer.c"

static struct timeval tv_exp1;
void callback1(time_evt_t *te, void *ptr)
{
    assert_int_equal(ptr, 1000);
    assert_int_equal(te->tv.tv_sec, tv_exp1.tv_sec);
    assert_int_equal(te->tv.tv_usec, tv_exp1.tv_usec);
}

static int ptr_exp2;
static struct timeval tv_exp2;
void callback2(time_evt_t *te, void *ptr)
{
    int r = ptr - NULL;

    assert_true(r == 0x100 || r == 0x200);
    ptr_exp2 |= r;

    assert_int_equal(te->tv.tv_sec, tv_exp2.tv_sec);
    assert_int_equal(te->tv.tv_usec, tv_exp2.tv_usec);
}

/// test few results known as good
static void test_t1(void **state)
{
    (void) state;   // unused

    ptr_exp2 = 0;
    tv_exp1.tv_sec = 0;
    tv_exp1.tv_usec = 0;
    tv_exp2.tv_sec = 0;
    tv_exp2.tv_usec = 0;

    tp_timer_t *timer = tp_timer_create();
    assert_non_null(timer);

    assert_true(tp_timer_register(timer, callback1, (void *)1000));
    tv_exp1.tv_usec = 1;
    tp_timer_tick(timer, false, 1);
    tp_timer_cancel(timer, callback1, (void *)1000);

    assert_true(tp_timer_register(timer, callback1, (void *)1000));
    assert_false(tp_timer_register(timer, callback1, (void *)1000));

    assert_true(tp_timer_register(timer, callback2, (void *)0x100));
    assert_true(tp_timer_register(timer, callback2, (void *)0x200));

    tp_timer_cancel(timer, callback1, (void *)1000);
    tp_timer_cancel(timer, callback1, (void *)1000);

    tv_exp1.tv_usec = -1;
    tv_exp2.tv_usec = 3;
    tp_timer_tick(timer, false, 2);
    assert_int_equal(ptr_exp2, 0x300);

    tp_timer_destroy(timer);
}

int main(void)
{
    const UnitTest tests[] = {
        unit_test(test_t1),
    };

    return run_tests(tests);
}
