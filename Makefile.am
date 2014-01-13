#******************************************************************************

include_HEADERS = \
	ydef.h ycrc.h ydynb.h ylistl.h ylist.h yqueue.h ystack.h \
	ytreel.h ytree.h yhash.h ytrie.h ymempool.h yset.h \
	ylib.h

pkglib_LTLIBRARIES = liby.la
liby_la_SOURCES = \
	crc.c tree.c trie.c hash.c mempool.c libmain.c

bin_PROGRAMS = y
y_SOURCES = \
	test_main.c test_list.c test_queue.c test_stack.c test_tree.c \
	test_hash.c test_mempool.c test_bitops.c test_set.c test_dynb.c
y_LDADD = liby.la
