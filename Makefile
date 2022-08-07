CC = clang
LDLIBS = -lwayland-client
all: client
client: client.c shm.h
	wayland-scanner client-header < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > xdg-shell.h
	wayland-scanner private-code < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > xdg-shell.c
	$(CC) $(LDLIBS) client.c xdg-shell.c -o client
	./client
