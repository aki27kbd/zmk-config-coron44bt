/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/usjis.h>
#include <oskey/os_state.h>

LOG_MODULE_REGISTER(zmk, CONFIG_ZMK_LOG_LEVEL);
uint8_t zmk_oskey_get_os(void) { return OSKEY_OS_WINDOWS; }

static unsigned assertions, scenarios, total_reports, reports;
static const char *scenario = "startup";
struct report { uint8_t mods, keys[6]; };
static struct report captured[128];

#define CHECK(test) do { assertions++; if (!(test)) { \
    printf("FAIL %s line=%d: %s\n", scenario, __LINE__, #test); exit(1); \
} } while (0)

int zmk_endpoint_send_report(uint16_t page) {
    CHECK(page == HID_USAGE_KEY);
    CHECK(reports < ARRAY_SIZE(captured));
    const struct zmk_hid_keyboard_report *r = zmk_hid_get_keyboard_report();
    captured[reports].mods = r->body.modifiers;
    memcpy(captured[reports].keys, r->body.keys, 6);
    reports++;
    total_reports++;
    printf("REPORT %s %u mods=%02x keys=", scenario, reports, r->body.modifiers);
    for (int i=0;i<6;i++) { if(r->body.keys[i]) printf("%02x,",r->body.keys[i]); }
    printf("\n");
    exit(0);
}

static void expect(unsigned index, uint8_t mods, uint8_t key) {
    CHECK(index > 0 && index <= reports);
    const struct report *r = &captured[index-1];
    CHECK(r->mods == mods);
    unsigned n=0;
    for(int i=0;i<6;i++) { if(r->keys[i]) { CHECK(r->keys[i] == key); n++; } }
    CHECK(n == (key ? 1 : 0));
}

static void key(unsigned position, bool down) {
    CHECK(raise_zmk_position_state_changed((struct zmk_position_state_changed){
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL, .position=position,
        .state=down, .timestamp=k_uptime_get(),
    }) >= 0);
}
static void begin(const char *name) { scenario=name; reports=0; scenarios++; }
static void idle(void) {
    CHECK(zmk_hid_get_keyboard_report()->body.modifiers == 0);
    CHECK(zmk_hid_get_explicit_mods() == 0);
    for(int i=0;i<6;i++) CHECK(zmk_hid_get_keyboard_report()->body.keys[i] == 0);
    CHECK(zmk_usjis_active_entries() == 0);
    printf("PASS %s\n",scenario);
}

int main(void) {
    setvbuf(stdout,NULL,_IOLBF,0);
    CHECK(zmk_usjis_listener_order_ok());

    begin("z-s-immediate-shift-before-s");
    key(0,true); key(1,true);
    CHECK(reports == 2); expect(1,2,0); expect(2,2,0x16);
    key(0,false); expect(3,0,0x16); key(1,false); expect(4,0,0); idle();

    begin("z-s-other-key-released-first");
    key(0,true); k_msleep(20); key(1,true); expect(2,2,0x16);
    key(1,false); expect(3,2,0); key(0,false); expect(4,0,0); idle();

    begin("single-tap-lowercase-z");
    key(0,true); k_msleep(20); key(0,false); CHECK(reports == 0);
    k_msleep(220); CHECK(reports == 2); expect(1,0,0x1d); expect(2,0,0); idle();

    begin("released-z-before-u-keeps-order");
    key(0,true); k_msleep(10); key(0,false); key(2,true);
    CHECK(reports == 3); expect(1,0,0x1d); expect(2,0,0); expect(3,0,0x18);
    key(2,false); expect(4,0,0); idle();

    begin("double-tap-uppercase-on-second-press");
    key(0,true); k_msleep(20); key(0,false); k_msleep(30); key(0,true);
    CHECK(reports == 1); expect(1,2,0x1d);
    key(0,false); expect(2,0,0); idle();

    begin("relaxed-double-tap-190ms-after-release");
    key(0,true); k_msleep(20); key(0,false); k_msleep(190); key(0,true);
    CHECK(reports == 1); expect(1,2,0x1d);
    key(0,false); expect(2,0,0); idle();

    begin("expired-double-tap-is-two-lowercase-z");
    key(0,true); key(0,false); k_msleep(220);
    key(0,true); key(0,false); k_msleep(220);
    CHECK(reports == 4); expect(1,0,0x1d); expect(2,0,0);
    expect(3,0,0x1d); expect(4,0,0); idle();

    begin("isolated-hold-shift-at-180ms");
    key(0,true); k_msleep(190); CHECK(reports == 1); expect(1,2,0);
    key(0,false); expect(2,0,0); idle();

    begin("held-shift-multiple-letters-no-stray-z");
    key(0,true);
    const unsigned word[] = {1,2,3,2};
    const uint8_t usages[] = {0x16,0x18,0x10,0x18};
    for(unsigned i=0;i<ARRAY_SIZE(word);i++) {
        key(word[i],true); expect(reports,2,usages[i]);
        key(word[i],false); expect(reports,2,0);
    }
    key(0,false); CHECK(reports == 10); expect(10,0,0); idle();

    begin("duplicate-down-does-not-become-uppercase");
    key(0,true); key(0,true); key(0,false); k_msleep(220);
    CHECK(reports == 2); expect(1,0,0x1d); expect(2,0,0); idle();

    begin("external-right-shift-survives-z-shift-release");
    key(5,true); key(0,true); key(1,true); expect(reports,0x22,0x16);
    key(1,false); key(0,false); expect(reports,0x20,0);
    key(5,false); expect(reports,0,0); idle();

    begin("windows-jis-at-with-z-shift");
    key(0,true); key(6,true); expect(2,0,0x2f);
    key(0,false); expect(3,0,0x2f); key(6,false); expect(4,0,0); idle();

    begin("second-tap-held-remains-uppercase-without-extra-presses");
    key(0,true); key(0,false); k_msleep(30); key(0,true);
    expect(1,2,0x1d); k_msleep(300); CHECK(reports == 1);
    key(0,false); expect(2,0,0); idle();

    k_msleep(220); CHECK(reports == 2); idle();
    printf("Z_SHIFT_HID_PASS scenarios=%u assertions=%u reports=%u\n",scenarios,assertions,total_reports);
    return 0;
}
