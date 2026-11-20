/* Connect host detection to oskey's public OS selection API. */

#include <cormoran/os-detection/os_detection.h>
#include <oskey/os_state.h>
#include <zmk/event_manager.h>

static int host_os_changed(const zmk_event_t *eh) {
    const struct zmk_os_changed *event = as_zmk_os_changed(eh);
    if (!event) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    switch (event->os) {
    case ZMK_OS_WINDOWS:
        zmk_oskey_set_os(OSKEY_OS_WINDOWS);
        break;
    case ZMK_OS_LINUX:
    case ZMK_OS_ANDROID:
        zmk_oskey_set_os(OSKEY_OS_LINUX);
        break;
    case ZMK_OS_MACOS:
    case ZMK_OS_IOS:
    case ZMK_OS_UNKNOWN:
    default:
        zmk_oskey_set_os(OSKEY_OS_MACOS);
        break;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(coron44bt_host_os, host_os_changed);
ZMK_SUBSCRIPTION(coron44bt_host_os, zmk_os_changed);
