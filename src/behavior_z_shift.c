/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT coron44bt_z_shift

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include "z_shift_state.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct z_shift_config {
    struct zmk_behavior_binding bindings[3];
    uint32_t hold_ms;
    uint32_t double_ms;
};

struct z_shift_data {
    const struct device *dev;
    struct z_shift_state state;
    struct zmk_behavior_binding_event origin;
    struct k_work_delayable timer;
};

/* Keep state transitions, emitted bindings and timer scheduling serialized.
 * Binding invocation raises keycode events, not position events. */
K_MUTEX_DEFINE(z_shift_mutex);

static void emit(const struct device *dev, unsigned index, bool down, int64_t now) {
    const struct z_shift_config *cfg = dev->config;
    struct z_shift_data *data = dev->data;
    struct zmk_behavior_binding_event event = data->origin;
    event.timestamp = now;
    int err = zmk_behavior_invoke_binding(&cfg->bindings[index], event, down);
    if (err < 0) {
        LOG_ERR("Z_SHIFT binding %u failed: %d", index, err);
    }
}

static void apply(const struct device *dev, enum z_shift_event event, int64_t now) {
    const struct z_shift_config *cfg = dev->config;
    struct z_shift_data *data = dev->data;
    unsigned actions = z_shift_step(&data->state, event, now, cfg->hold_ms, cfg->double_ms);

    if (actions & ZS_TAP_Z) {
        emit(dev, 1, true, now);
        emit(dev, 1, false, now);
    }
    if (actions & ZS_SHIFT_DOWN) { emit(dev, 0, true, now); }
    if (actions & ZS_SHIFT_UP) { emit(dev, 0, false, now); }
    if (actions & ZS_UPPER_DOWN) { emit(dev, 2, true, now); }
    if (actions & ZS_UPPER_UP) { emit(dev, 2, false, now); }

    if (data->state.phase == ZS_PRESSED || data->state.phase == ZS_TAP_PENDING) {
        int64_t remaining = data->state.deadline - k_uptime_get();
        k_work_reschedule(&data->timer, K_MSEC(MAX(remaining, 1)));
    } else {
        k_work_cancel_delayable(&data->timer);
    }
}

static void timeout(struct k_work *work) {
    struct z_shift_data *data = CONTAINER_OF(k_work_delayable_from_work(work),
                                            struct z_shift_data, timer);
    k_mutex_lock(&z_shift_mutex, K_FOREVER);
    apply(data->dev, ZS_TIMER, k_uptime_get());
    k_mutex_unlock(&z_shift_mutex);
}

static int pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct z_shift_data *data = dev->data;
    k_mutex_lock(&z_shift_mutex, K_FOREVER);
    if (data->state.phase != ZS_IDLE && data->origin.position != event.position) {
        k_mutex_unlock(&z_shift_mutex);
        return -EBUSY;
    }
    data->origin = event;
    apply(dev, ZS_PRESS, event.timestamp);
    k_mutex_unlock(&z_shift_mutex);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct z_shift_data *data = dev->data;
    k_mutex_lock(&z_shift_mutex, K_FOREVER);
    if (data->origin.position == event.position) {
        apply(dev, ZS_RELEASE, event.timestamp);
    }
    k_mutex_unlock(&z_shift_mutex);
    return ZMK_BEHAVIOR_OPAQUE;
}

static void other_key(const struct device *dev, const struct zmk_position_state_changed *event) {
    struct z_shift_data *data = dev->data;
    if (!device_is_ready(dev)) { return; }
    k_mutex_lock(&z_shift_mutex, K_FOREVER);
    if (data->state.phase != ZS_IDLE && data->origin.position != event->position) {
        apply(dev, ZS_OTHER_PRESS, event->timestamp);
    }
    k_mutex_unlock(&z_shift_mutex);
}

static int position_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event && event->state) {
#define OTHER_KEY(n) other_key(DEVICE_DT_INST_GET(n), event);
        DT_INST_FOREACH_STATUS_OKAY(OTHER_KEY)
#undef OTHER_KEY
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(coron44bt_z_shift, position_listener);
ZMK_SUBSCRIPTION(coron44bt_z_shift, zmk_position_state_changed);

static int init(const struct device *dev) {
    struct z_shift_data *data = dev->data;
    data->dev = dev;
    data->origin.position = UINT32_MAX;
    k_work_init_delayable(&data->timer, timeout);
    return 0;
}

static const struct behavior_driver_api api = {
    .binding_pressed = pressed,
    .binding_released = released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define Z_SHIFT_INST(n)                                                                            \
    BUILD_ASSERT(DT_INST_PROP_LEN(n, bindings) == 3, "Z_SHIFT requires three bindings");           \
    BUILD_ASSERT(DT_INST_PROP(n, hold_term_ms) > 0, "Z_SHIFT hold term must be positive");         \
    BUILD_ASSERT(DT_INST_PROP(n, double_tap_ms) > 0, "Z_SHIFT double tap term must be positive");   \
    static struct z_shift_data data_##n;                                                           \
    static const struct z_shift_config config_##n = {                                              \
        .bindings = {ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                                \
                     ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                                \
                     ZMK_KEYMAP_EXTRACT_BINDING(2, DT_DRV_INST(n))},                               \
        .hold_ms = DT_INST_PROP(n, hold_term_ms),                                                   \
        .double_ms = DT_INST_PROP(n, double_tap_ms),                                                \
    };                                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, init, NULL, &data_##n, &config_##n, POST_KERNEL,                     \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);

DT_INST_FOREACH_STATUS_OKAY(Z_SHIFT_INST)
