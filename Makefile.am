#******************************************************************************

include_HEADERS = \
	ydef.h \
	ycommon.h \
	ycrc.h \
	ydynb.h \
	ylistl.h \
	ylist.h \
	ytreel.h \
	ytree.h \
	yhash.h \
	ytrie.h \
	ymempool.h \
	yset.h \
	ylru.h \
	ymsgq.h \
	ystatmath.h \
	ystatprint.h \
	ygraph.h \
	ymtconcur.h \
	ylib.h

pkglib_LTLIBRARIES = liby.la
liby_la_SOURCES = \
	crc.c \
	list.c \
	tree.c \
	trie.c \
	hash.c \
	mempool.c \
	lru.c \
	msgq.c \
	statprint.c \
	graph.c \
	p.c \
	mtconcur.c \
	libmain.c

bin_PROGRAMS = y
y_SOURCES = \
	test_main.c \
	test_list.c \
	test_tree.c \
	test_hash.c \
	test_mempool.c \
	test_bitops.c \
	test_dynb.c \
	test_p.c \
	test_lru.c \
	test_statmath.c \
	test_statprint.c \
	test_graph.c \
	test_etc.c \
	test_mtconcur.c


y_LDADD = liby.la -lpthread -lrt -lm

