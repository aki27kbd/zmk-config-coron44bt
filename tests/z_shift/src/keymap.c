/* SPDX-License-Identifier: MIT */
/* The hardware/keymap boundary: dispatch positions through the real behavior
 * API. This listener is placed after Z_SHIFT, just as in the firmware. */
#include <zephyr/device.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <dt-bindings/zmk/keys.h>

static const struct zmk_behavior_binding keys[] = {
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(z_shift))},
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), .param1 = S},
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), .param1 = U},
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), .param1 = M},
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), .param1 = MINUS},
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), .param1 = RSHFT},
    {.behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), .param1 = N2},
};

uint32_t zmk_keymap_layer_state(void) { return 1; }

static int dispatch(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (!event || event->position >= ARRAY_SIZE(keys)) { return ZMK_EV_EVENT_BUBBLE; }
    struct zmk_behavior_binding_event binding_event = {
        .position = event->position, .timestamp = event->timestamp,
    };
    return zmk_behavior_invoke_binding(&keys[event->position], binding_event, event->state);
}

ZMK_LISTENER(keymap, dispatch);
ZMK_SUBSCRIPTION(keymap, zmk_position_state_changed);
