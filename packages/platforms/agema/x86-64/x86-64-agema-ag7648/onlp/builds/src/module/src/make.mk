###############################################################################
#
# 
#
###############################################################################

LIBRARY := x86_64_agema_ag7648
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk