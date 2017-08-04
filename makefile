# TODO: actually make a proper makefile .-.
ckcc:
	gcc ckcc.c -o ckcc_bin -Wall -Wextra -pedantic -O2 -std=c99 -lusb
ckcc_debug:
	gcc ckcc.c -o ckcc_bin -Wall -Wextra -pedantic -O0 -std=c99 -lusb \
	-fstack-protector-all -g -D U_DBG
clean:
	rm ckcc_bin
