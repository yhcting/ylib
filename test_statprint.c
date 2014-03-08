/******************************************************************************
 *    Copyright (C) 2011, 2012, 2013, 2014
 *    Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
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

/**
 * Linked list test.
 */
static void
test_statprint(void) {
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
	unsigned int ix0[]
		= {};
	unsigned int ix1[]
		= {0};
	unsigned int ix2[]
		= {0, 1, 1000};
	unsigned int ix3[]
		= {0, 2, 4};
	unsigned int ix4[]
		= {0, 0, 2, 4, 4};
	unsigned int ix5[]
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

TESTFN(test_statprint, statprint)

