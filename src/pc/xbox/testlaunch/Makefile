# xemu test aid for the network loaders (tools/xemu_loader_test.sh): make -C src/pc/xbox/testlaunch
# (after . ~/xboxdev/env.sh) -> build/xbox/testlaunch/default.xbe. NEVER `make clean` (it deletes nxdk's libs).
XBE_TITLE = testlaunch
SRCS = main.c
OUTPUT_DIR = ../../../../build/xbox/testlaunch
include $(NXDK_DIR)/Makefile
