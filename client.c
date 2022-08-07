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

void bufRelease(void *data, struct wl_buffer *wl_buffer){
    wl_buffer_destroy(wl_buffer);
}

struct wl_buffer_listener bufListener = {bufRelease};

void draw_frame();

void surfaceConf(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
  xdg_surface_ack_configure(xdg_surface, serial);
  draw_frame();
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
}

void regDel(void *data, struct wl_registry *wl_registry, uint32_t name) {

}

static const struct wl_registry_listener regListener = {regAdd, regDel};

void draw_frame() {
  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
  wl_buffer_add_listener(buffer, &bufListener, NULL);
  wl_surface_attach(wl_surface, buffer, 0, 0);
  wl_surface_commit(wl_surface);
}

void init() {
  wl_display = wl_display_connect(NULL);
  wl_registry = wl_display_get_registry(wl_display);
  wl_registry_add_listener(wl_registry, &regListener, NULL);
  wl_display_roundtrip(wl_display);

  wl_surface = wl_compositor_create_surface(wl_compositor);
  xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, wl_surface);
  xdg_surface_add_listener(xdg_surface, &surfaceListener, NULL);

  fd = allocate_shm_file(size);
  pool = wl_shm_create_pool(wl_shm, fd, size);
  data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
  wl_surface_commit(wl_surface);
}

int main() {
  init();
  for (int i = 0; i < height * width; i++) 
    data[i] = i*999;
  while (wl_display_dispatch(wl_display)) {
  }
  return 0;
}
