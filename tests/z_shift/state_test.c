/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "z_shift_state.h"

static unsigned checks;
#define EXPECT(s, event, time, actions, next) do { \
    unsigned actual = z_shift_step(&(s), event, time, 180, 200); \
    assert(actual == (actions)); assert((s).phase == (next)); checks += 2; \
} while (0)

int main(void) {
    /* Even a zero-delay interrupt must establish Shift before the next key. */
    const int delays[] = {0, 1, 20, 100, 179};
    for (unsigned i = 0; i < sizeof(delays)/sizeof(delays[0]); i++) {
        struct z_shift_state s = {0};
        EXPECT(s, ZS_PRESS, 0, 0, ZS_PRESSED);
        EXPECT(s, ZS_OTHER_PRESS, delays[i], ZS_SHIFT_DOWN, ZS_SHIFT_HELD);
        EXPECT(s, ZS_OTHER_PRESS, 200, 0, ZS_SHIFT_HELD);
        EXPECT(s, ZS_RELEASE, 220, ZS_SHIFT_UP, ZS_IDLE);
        EXPECT(s, ZS_TIMER, 500, 0, ZS_IDLE);
    }

    struct z_shift_state s = {0};
    EXPECT(s, ZS_PRESS, 0, 0, ZS_PRESSED);
    EXPECT(s, ZS_RELEASE, 30, 0, ZS_TAP_PENDING);
    EXPECT(s, ZS_OTHER_PRESS, 40, ZS_TAP_Z, ZS_IDLE);
    EXPECT(s, ZS_TIMER, 500, 0, ZS_IDLE);

    s = (struct z_shift_state){0};
    EXPECT(s, ZS_PRESS, 0, 0, ZS_PRESSED);
    EXPECT(s, ZS_TIMER, 179, 0, ZS_PRESSED);
    EXPECT(s, ZS_TIMER, 180, ZS_SHIFT_DOWN, ZS_SHIFT_HELD);
    EXPECT(s, ZS_TIMER, 500, 0, ZS_SHIFT_HELD);
    EXPECT(s, ZS_RELEASE, 501, ZS_SHIFT_UP, ZS_IDLE);

    /* Double-tap time is independent of the isolated hold timeout. */
    for (int gap = 0; gap < 200; gap++) {
        s = (struct z_shift_state){0};
        EXPECT(s, ZS_PRESS, 0, 0, ZS_PRESSED);
        EXPECT(s, ZS_RELEASE, 100, 0, ZS_TAP_PENDING);
        EXPECT(s, ZS_PRESS, 100+gap, ZS_UPPER_DOWN, ZS_UPPER_HELD);
        EXPECT(s, ZS_TIMER, 600+gap, 0, ZS_UPPER_HELD);
        EXPECT(s, ZS_RELEASE, 700+gap, ZS_UPPER_UP, ZS_IDLE);
    }

    s = (struct z_shift_state){0};
    EXPECT(s, ZS_PRESS, 0, 0, ZS_PRESSED);
    EXPECT(s, ZS_RELEASE, 20, 0, ZS_TAP_PENDING);
    EXPECT(s, ZS_TIMER, 219, 0, ZS_TAP_PENDING);
    EXPECT(s, ZS_TIMER, 220, ZS_TAP_Z, ZS_IDLE);
    EXPECT(s, ZS_PRESS, 230, 0, ZS_PRESSED);
    EXPECT(s, ZS_RELEASE, 250, 0, ZS_TAP_PENDING);
    EXPECT(s, ZS_PRESS, 450, ZS_TAP_Z, ZS_PRESSED);
    EXPECT(s, ZS_RELEASE, 640, ZS_SHIFT_DOWN | ZS_SHIFT_UP, ZS_IDLE);

    /* Duplicate press notifications do not count as a second tap. */
    s = (struct z_shift_state){0};
    EXPECT(s, ZS_PRESS, 0, 0, ZS_PRESSED);
    EXPECT(s, ZS_PRESS, 10, 0, ZS_PRESSED);
    EXPECT(s, ZS_RELEASE, 20, 0, ZS_TAP_PENDING);
    EXPECT(s, ZS_RELEASE, 30, 0, ZS_TAP_PENDING);
    EXPECT(s, ZS_TIMER, 221, ZS_TAP_Z, ZS_IDLE);
    EXPECT(s, ZS_RELEASE, 250, 0, ZS_IDLE);
    printf("Z_SHIFT_STATE_PASS assertions=%u\n", checks);
    return 0;
}
