# -------------------------------------------------------------------
# vi: set noet ts=4 sw=4 fenc=utf-8
# -------------------------------------------------------------------
# Copyright (c) 2025 Francis Henry Grolemund III and RealProven, LLC. 
# SPDX-License-Identifier: LicenseRef-Unum-Commercial OR GPL-3.0-only
# -------------------------------------------------------------------
.PHONY : all clean clean-test

BASIS  := ./.unum
UBOOT  := $(BASIS)/deployed/bin/uboot
MKDIR  := mkdir -p
RMDIR  := rm -rf

all : $(UBOOT)
	@$(UBOOT) --cpp=$(CXX) --link=$(LD)
	@$(BASIS)/deployed/bin/unum deploy

clean:
	$(RMDIR) $(BASIS)/deployed

clean-test:
	$(RMDIR) $(BASIS)/deployed/test

$(UBOOT): $(BASIS)/boot/main.cc
	$(MKDIR) $(BASIS)/deployed/bin
	$(CXX) -o $@ $^



