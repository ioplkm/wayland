#include <xkbcommon/xkbcommon.h>
struct wl_seat *seat;
struct wl_keyboard *keyboard;
struct xkb_state *xkb_state;
struct xkb_context *xkb_context;
struct xkb_keymap *xkb_keymap;

void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
  char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  xkb_keymap = xkb_keymap_new_from_string(xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm, size);
  close(fd);
  xkb_state = xkb_state_new(xkb_keymap);
  //xkb_keymap_unref(xkb_keymap);
  //xkb_state_unref(xkb_state);
}

void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
  printf("key pressed:\n");
  uint32_t *key;
  wl_array_for_each(key, keys) {
    char buf[128];
    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, *key + 8);
    xkb_keysym_get_name(sym, buf, sizeof(buf));
    printf("sym: %-12s (%d), ", buf, sym);
    xkb_state_key_get_utf8(xkb_state, *key + 8, buf, sizeof(buf));
    printf("utf8: '%s'\n", buf);
  }
}

void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
  char buf[128];
  uint32_t keycode = key + 8;
  xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, keycode);
  xkb_keysym_get_name(sym, buf, sizeof(buf));
  char *action = state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
  printf("key %s: sym: %-12s (%d), ", action, buf, sym);
  xkb_state_key_get_utf8(xkb_state, keycode, buf, sizeof(buf));
  printf("utf8: '%s'\n", buf);
}

void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
  printf("keyboard leave\n");
}

void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
  xkb_state_update_mask(xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
  //
}

struct wl_keyboard_listener kbList = {wl_keyboard_keymap, wl_keyboard_enter, wl_keyboard_leave, wl_keyboard_key, wl_keyboard_modifiers, wl_keyboard_repeat_info};

void seatCap(void *data, struct wl_seat *wl_seat, uint32_t caps) {
  if (keyboard == NULL) {
    keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(keyboard, &kbList, NULL);
  }
}

void seatName(void *data, struct wl_seat *wl_seat, const char *name) {
  printf("seat name: %s\n", name);
}

struct wl_seat_listener seatList = {seatCap, seatName};

