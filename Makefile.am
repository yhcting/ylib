#******************************************************************************

include_HEADERS = \
	ydef.h yret.h ycrc.h ydynb.h ylistl.h ylist.h yqueue.h ystack.h \
	ytreel.h ytree.h yhash.h ytrie.h ymempool.h \
	ylib.h

pkglib_LTLIBRARIES = liby.la
liby_la_SOURCES = \
	crc.c tree.c trie.c hash.c ret.c mempool.c

bin_PROGRAMS = y
y_SOURCES = \
	test_main.c test_list.c test_queue.c test_stack.c test_tree.c \
	test_hash.c test_mempool.c
y_LDADD = liby.la
