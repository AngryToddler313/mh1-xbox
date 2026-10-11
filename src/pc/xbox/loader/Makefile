# MH1 network loader: make -C src/pc/xbox/loader (after . ~/xboxdev/env.sh)
# -> build/xbox/loader/default.xbe; tools/xbox_upload.sh --loader installs it.
XBE_TITLE = Monster\ Hunter
SRCS = main.c ../xnet_http.c
NXDK_NET = y
OUTPUT_DIR = ../../../../build/xbox/loader
include $(NXDK_DIR)/Makefile
