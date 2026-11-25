/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdint.h>

enum z_shift_phase { ZS_IDLE, ZS_PRESSED, ZS_TAP_PENDING, ZS_SHIFT_HELD, ZS_UPPER_HELD };
enum z_shift_event { ZS_PRESS, ZS_RELEASE, ZS_OTHER_PRESS, ZS_TIMER };
enum z_shift_action {
    ZS_SHIFT_DOWN = 1u << 0,
    ZS_SHIFT_UP = 1u << 1,
    ZS_TAP_Z = 1u << 2,
    ZS_UPPER_DOWN = 1u << 3,
    ZS_UPPER_UP = 1u << 4,
};

struct z_shift_state {
    enum z_shift_phase phase;
    int64_t deadline;
};

/* Pure transition policy. Actions are emitted before the interrupted keymap
 * listener runs. A real release is required for a double tap. */
static inline unsigned z_shift_step(struct z_shift_state *s, enum z_shift_event event,
                                    int64_t now, uint32_t hold_ms, uint32_t double_ms) {
    unsigned actions = 0;
    if (now >= s->deadline) {
        if (s->phase == ZS_PRESSED) {
            s->phase = ZS_SHIFT_HELD;
            actions |= ZS_SHIFT_DOWN;
        } else if (s->phase == ZS_TAP_PENDING) {
            s->phase = ZS_IDLE;
            actions |= ZS_TAP_Z;
        }
    }

    switch (event) {
    case ZS_PRESS:
        if (s->phase == ZS_TAP_PENDING) {
            s->phase = ZS_UPPER_HELD;
            actions |= ZS_UPPER_DOWN;
        } else if (s->phase == ZS_IDLE) {
            s->phase = ZS_PRESSED;
            s->deadline = now + hold_ms;
        }
        break;
    case ZS_RELEASE:
        if (s->phase == ZS_PRESSED) {
            s->phase = ZS_TAP_PENDING;
            s->deadline = now + double_ms;
        } else if (s->phase == ZS_SHIFT_HELD) {
            s->phase = ZS_IDLE;
            actions |= ZS_SHIFT_UP;
        } else if (s->phase == ZS_UPPER_HELD) {
            s->phase = ZS_IDLE;
            actions |= ZS_UPPER_UP;
        }
        break;
    case ZS_OTHER_PRESS:
        if (s->phase == ZS_PRESSED) {
            s->phase = ZS_SHIFT_HELD;
            actions |= ZS_SHIFT_DOWN;
        } else if (s->phase == ZS_TAP_PENDING) {
            s->phase = ZS_IDLE;
            actions |= ZS_TAP_Z;
        }
        break;
    case ZS_TIMER:
        break;
    }
    return actions;
}
