#include <string.h>

#include <wayland-client.h>

#include "xdg-shell.h"
#include "shm.h"

#define width 960
#define height 1056
int stride = width * 4;
int size = width * 4 * height;

int fd;
uint32_t *data;

struct wl_display *wl_display;
struct wl_registry *wl_registry;
struct wl_shm *wl_shm;
struct wl_shm_pool *pool;
struct wl_buffer *buffer;
struct wl_compositor *wl_compositor;
struct wl_surface *wl_surface;
struct xdg_wm_base *xdg_wm_base;
struct xdg_surface *xdg_surface;
struct xdg_toplevel *xdg_toplevel;
struct wl_callback *callback;

#include "kbd.h"

void bufRelease(void *data, struct wl_buffer *wl_buffer){
    wl_buffer_destroy(wl_buffer);
}

struct wl_buffer_listener bufListener = {bufRelease};

void surfaceConf(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
  xdg_surface_ack_configure(xdg_surface, serial);
  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
  
  wl_surface_attach(wl_surface, buffer, 0, 0);
  wl_surface_commit(wl_surface);
}

struct xdg_surface_listener surfaceListener = {surfaceConf};

void xdgPing(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

struct xdg_wm_base_listener xdgListener = {xdgPing};

void regAdd(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version) {
  if (strcmp(interface, wl_shm_interface.name) == 0) {
      wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
      wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
      xdg_wm_base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
      xdg_wm_base_add_listener(xdg_wm_base, &xdgListener, NULL);
  }
  //keyboard
  else if (strcmp(interface, wl_seat_interface.name) == 0) {
    seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
    wl_seat_add_listener(seat, &seatList, NULL);
  }
}

void regDel(void *data, struct wl_registry *wl_registry, uint32_t name) {

}

struct wl_registry_listener regListener = {regAdd, regDel};

struct wl_callback_listener surfaceFrameListener;

void surfaceCallback(void *data, struct wl_callback *callback, uint32_t time) {
  wl_callback_destroy(callback);
  callback = wl_surface_frame(wl_surface);
  wl_callback_add_listener(callback, &surfaceFrameListener, NULL);

  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
  wl_buffer_add_listener(buffer, &bufListener, NULL);
  wl_surface_attach(wl_surface, buffer, 0, 0);
  wl_surface_damage_buffer(wl_surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit(wl_surface);
};

struct wl_callback_listener surfaceFrameListener = {surfaceCallback};

void init() {
  wl_display = wl_display_connect(NULL);
  wl_registry = wl_display_get_registry(wl_display);
  wl_registry_add_listener(wl_registry, &regListener, NULL);
  wl_display_roundtrip(wl_display);

  wl_surface = wl_compositor_create_surface(wl_compositor);
  xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, wl_surface);
  xdg_surface_add_listener(xdg_surface, &surfaceListener, NULL);
  xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
  wl_surface_commit(wl_surface);

  fd = allocate_shm_file(size);
  pool = wl_shm_create_pool(wl_shm, fd, size);
  data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  callback = wl_surface_frame(wl_surface);
  wl_callback_add_listener(callback, &surfaceFrameListener, NULL);
}

void end() {
  munmap(data, size);
  wl_shm_pool_destroy(pool);
  close(fd);
}

int main() {
  xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  init();
  uint32_t a = 0;
  while (wl_display_dispatch(wl_display)) {
    for (int i = 0; i < width * height; i++) {
      data[i] = a;
    }
      a+=0x00010001;
  }
  end();
  return 0;
}
