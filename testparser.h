/* bmplibtest - testparser.h
 *
 * Copyright (c) 2025, Rupert Weber.
 *
 * This file is part of bmplibtest.
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


struct Argument {
	struct Argument *next;
	char            *argname;
	char            *argvalue;
};

struct Action {
	struct Action   *next;
	char            *actname;
	struct Argument *arglist;
};

enum CommandType {
	COMMAND_TEST,
	COMMAND_SETTINGS,
};

struct Command {
	struct Command  *next;
	enum CommandType type;
	char            *descr;
	struct Action   *actionlist;
};

enum TestPrintStyle {
	PRINTSTYLE_DUMP,
	PRINTSTYLE_PRETTY,
};

struct Command* parse_test_definitions(FILE *file);
void print_test_definitions(enum TestPrintStyle style);
void free_cmdlist(void);
