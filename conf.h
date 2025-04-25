/* bmplibtest - conf.h
 *
 * Copyright (c) 2024-2025, Rupert Weber.
 *
 * This file is part of bmplibtest.
 *
 * bmplibtest is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

struct Confstr {
	struct Confstr *next;
	char            str[1];
};

struct Conf {
	int             verbose;
	char           *sampledir;
	char           *refdir;
	char           *tmpdir;
	char           *testfile;
	bool            env;
	bool            help;
	bool            dump;
	bool            pretty;
	int             nstrings;
	struct Confstr *strlist;
};

struct Conf* conf_parse_cmdline(int argc, char **argv);
void conf_free(struct Conf *cmdline);
void conf_usage(void);
