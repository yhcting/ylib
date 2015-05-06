/******************************************************************************
 * Copyright (C) 2011, 2012, 2013, 2014
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include "ycommon.h"
#include "test.h"
#include "ystatprint.h"

static void
test_bargraph(void) {
	int fd, r;
	const char *expected_result;
	char buf[16384];
	char *p;
	const char *tmpf = "____temp__";
	double v0[]
		= { };
	double v1[]
		= {-1, -3 };
	double v2[]
		= {1.0f, 2.5f, 0.0f, 5.0f, -1.0f, 9.0f, 0.0f };
	u32 ix0[]
		= {};
	u32 ix1[]
		= {0};
	u32 ix2[]
		= {0, 1, 1000};
	u32 ix3[]
		= {0, 2, 4};
	u32 ix4[]
		= {0, 0, 2, 4, 4};
	u32 ix5[]
		= {0, 0, 0, 2, 4, 4};
	const char *is[]
		= {"is0", "is1", "is2", "is3", "is4", "is5" };

	fd = open(tmpf, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	r = ystpr_bargraph(fd,
			   v0,
			   0,
			   10,
			   NULL,
			   NULL,
			   0,
			   0,
			   '*');
	close(fd);
	yassert(r);

	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v1,
			   arrsz(v1),
			   10,
			   NULL,
			   NULL,
			   0,
			   1,
			   '*');
	close(fd);
	yassert(!r);
	expected_result =
"---- -1.000000\n"
"*   \n"
"*   \n"
"*   \n"
"*   \n"
"*   \n"
"*   \n"
"*   \n"
"*   \n"
"*   \n"
"* * \n"
"---- -3.000000\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v2,
			   arrsz(v2),
			   20,
			   ix2,
			   is,
			   arrsz(ix2),
			   1,
			   '*');
	close(fd);
	yassert(r);

	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v2,
			   arrsz(v2),
			   20,
			   ix0,
			   is,
			   arrsz(ix0),
			   1,
			   '*');
	close(fd);
	yassert(!r);
	expected_result =
"-------------- 9.000000\n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"* *   *   *   \n"
"* *   *   *   \n"
"* * * *   * * \n"
"* * * *   * * \n"
"* * * * * * * \n"
"-------------- -1.000000\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));

	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v2,
			   arrsz(v2),
			   20,
			   ix1,
			   is,
			   arrsz(ix1),
			   1,
			   '*');
	close(fd);
	yassert(!r);
	expected_result =
"-------------- 9.000000\n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"* *   *   *   \n"
"* *   *   *   \n"
"* * * *   * * \n"
"* * * *   * * \n"
"* * * * * * * \n"
"-------------- -1.000000\n"
"^\n"
"is0\n"
"\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	{
		int i = 0;
		for (i = 0; i < strlen(buf); i++) {
			if (buf[i] != expected_result[i]) {
				printf("*** %d : [%s]\n", i, &buf[i]);
			}
		}
	}
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v2,
			   arrsz(v2),
			   20,
			   ix3,
			   is,
			   arrsz(ix3),
			   1,
			   '*');
	close(fd);
	yassert(!r);
	expected_result =
"-------------- 9.000000\n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"* *   *   *   \n"
"* *   *   *   \n"
"* * * *   * * \n"
"* * * *   * * \n"
"* * * * * * * \n"
"-------------- -1.000000\n"
"^   ^   ^\n"
"is0     is2\n"
"    is1\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v2,
			   arrsz(v2),
			   20,
			   ix4,
			   is,
			   arrsz(ix4),
			   1,
			   '*');
	close(fd);
	yassert(!r);
	expected_result =
"-------------- 9.000000\n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"* *   *   *   \n"
"* *   *   *   \n"
"* * * *   * * \n"
"* * * *   * * \n"
"* * * * * * * \n"
"-------------- -1.000000\n"
"^   ^   ^\n"
"is0 is2 is4\n"
"is1     is3\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_bargraph(fd,
			   v2,
			   arrsz(v2),
			   20,
			   ix5,
			   is,
			   arrsz(ix5),
			   1,
			   '*');
	close(fd);
	yassert(!r);
	expected_result =
"-------------- 9.000000\n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"          *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"      *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"  *   *   *   \n"
"* *   *   *   \n"
"* *   *   *   \n"
"* * * *   * * \n"
"* * * *   * * \n"
"* * * * * * * \n"
"-------------- -1.000000\n"
"^   ^   ^\n"
"is2     is4\n"
"is1 is3 is5\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	unlink(tmpf);
}

static void
test_distgraph(void) {
	int fd, r;
	const char *expected_result;
	char buf[16384];
	char *p;
	const char *tmpf = "____temp__";
	double v0[] = {};
	double v1[] = {1.0f};
	double v2[] = {1.0f, 2.0f, 1.0f, 1.0f};
	double v3[] = {
		1.0f, 2.5f, 0.0f, 5.0f, -1.0f, 9.0f, 0.0f, 1.5f, 2.0f,
		2.3f, 1.2f, 1.4f, 1.9f, 2.7f};

	fd = open(tmpf, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	r = ystpr_distgraph(fd,
			    v0,
			    arrsz(v0),
			    20,
			    15,
			    1,
			    '*');
	close(fd);
	yassert(!r);
	expected_result = "All values are same.\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_distgraph(fd,
			    v1,
			    arrsz(v1),
			    20,
			    15,
			    1,
			    '*');
	close(fd);
	yassert(!r);
	expected_result = "All values are same.\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	r = ystpr_distgraph(fd,
			    v2,
			    arrsz(v2),
			    20,
			    15,
			    1,
			    '*');
	close(fd);
	yassert(!r);
	expected_result =
"---------------------------------------- 3.000000\n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                       \n"
"*                                     * \n"
"*                                     * \n"
"*                                     * \n"
"*                                     * \n"
"*                                     * \n"
"* * * * * * * * * * * * * * * * * * * * \n"
"---------------------------------------- 0.000000\n"
"^         ^               ^           ^\n"
"1.000000                  1s\n"
"          u                           2.000000\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));


	fd = open(tmpf, O_WRONLY | O_TRUNC);
	ystpr_distgraph(fd,
			v3,
			arrsz(v3),
			20,
			15,
			1,
			'*');
	close(fd);
	yassert(!r);
	expected_result =
"---------------------------------------- 3.000000\n"
"        *                               \n"
"        *                               \n"
"        *                               \n"
"        *                               \n"
"    *   * * * *                         \n"
"    *   * * * *                         \n"
"    *   * * * *                         \n"
"    *   * * * *                         \n"
"    *   * * * *                         \n"
"*   *   * * * *         *             * \n"
"*   *   * * * *         *             * \n"
"*   *   * * * *         *             * \n"
"*   *   * * * *         *             * \n"
"*   *   * * * *         *             * \n"
"* * * * * * * * * * * * * * * * * * * * \n"
"---------------------------------------- 0.000000\n"
"^ ^         ^       ^         ^       ^\n"
"-1.000000   u                 2s\n"
"  -1s               1s                9.000000\n";
	memset(buf, 0, sizeof(buf));
	p = buf;
	fd = open(tmpf, O_RDONLY);
	while (0 < (r = read(fd, p, sizeof(buf) - (p - buf))))
		p += r;
	close(fd);
	yassert(!strcmp(expected_result, buf));

	unlink(tmpf);
}

static void
test_statprint(void) {
	test_bargraph();
	test_distgraph();
}

TESTFN(test_statprint, statprint)

