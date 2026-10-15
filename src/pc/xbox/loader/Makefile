# MH1 network loader: make -C src/pc/xbox/loader (after . ~/xboxdev/env.sh)
# -> build/xbox/loader/default.xbe; tools/xbox_upload.sh --loader installs it.
XBE_TITLE = Monster\ Hunter
SRCS = main.c install.c ../xnet_http.c
NXDK_NET = y
OUTPUT_DIR = ../../../../build/xbox/loader
# XH_HOST / XH_PORT (optional): the laptop address used when loader.txt is missing
# (default 192.168.1.166:8765). A test copy for xemu (tools/xemu_loader_test.sh):
#   make OUTPUT_DIR=../../../../build/xbox/loader_test XH_HOST='\"10.0.2.2\"' XH_PORT=8799
# then delete ../xnet_http.obj and main.obj, or the next plain make links the test address.
# NEVER `make clean` here: nxdk's clean also deletes the shared nxdk libraries in
# ~/xboxdev/nxdk/lib (13 Oct 2026; rebuilt by building any nxdk program with NXDK_NET,
# NXDK_SDL and NXDK_CXX = y).
ifneq ($(XH_PORT),)
CFLAGS += -DXH_PORT=$(XH_PORT)
endif
ifneq ($(XH_HOST),)
CFLAGS += -DXH_HOST=$(XH_HOST)
endif
include $(NXDK_DIR)/Makefile
