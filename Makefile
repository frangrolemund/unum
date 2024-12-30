# --------------------------------------------------------------
# vi: set noet ts=4 sw=4 fenc=utf-8
# ---------------------------------------------------------------
# Copyright 2024 Francis Henry Grolemund III
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
# PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
# --------------------------------------------------------------
.PHONY : all clean clean-test

BASIS  := ./.unum
UBOOT  := $(BASIS)/deployed/bin/uboot
MKDIR  := mkdir -p
RMDIR  := rm -rf

all : $(UBOOT)
	@$(UBOOT) --cc=$(CC) --ld=$(LD) 
	@$(BASIS)/deployed/bin/unum deploy

clean :
	$(RMDIR) $(BASIS)/deployed

clean-test:
	$(RMDIR) $(BASIS)/deployed/test

$(UBOOT): $(BASIS)/boot/main.c
	$(MKDIR) $(BASIS)/deployed/bin
	$(CC) -o $@ $^



