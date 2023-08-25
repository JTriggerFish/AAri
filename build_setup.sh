#Initial setup
meson setup debug --buildtype=debug --backend=ninja
meson setup release --buildtype=release --backend=ninja

#Testing:
meson test -C debug