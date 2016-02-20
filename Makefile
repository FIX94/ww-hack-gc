# Copyright 2016 FIX94
# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file LICENSE or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

all:
	@$(MAKE) --no-print-directory -C loader
	@mv -f loader/loader.bin exploit/loader.bin
	@$(MAKE) --no-print-directory -C exploit
	@mkdir -p gci
	@mv -f exploit/*.gci gci

clean:
	@$(MAKE) --no-print-directory -C loader clean
	@$(MAKE) --no-print-directory -C exploit clean
	rm -rf gci
