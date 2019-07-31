/*
Minetest
Copyright (C) 2014 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include "config.h"

#if ENABLE_MYSQL

#include <mysql.h>
#include <string>
#include "database.h"

class Settings;

class Database_MySQL : public MapDatabase
{
public:
	Database_MySQL(Settings &conf);
	~Database_MySQL();

	void beginSave();
	void endSave();

	bool saveBlock(const v3s16 &pos, const std::string &data);
	void loadBlock(const v3s16 &pos, std::string *block);
	bool deleteBlock(const v3s16 &pos);
	void listAllLoadableBlocks(std::vector<v3s16> &dst);

private:
	void initStatements();

	MYSQL *connection = nullptr;

	MYSQL_STMT *stmt_read = nullptr;
	MYSQL_STMT *stmt_write = nullptr;
	MYSQL_STMT *stmt_list = nullptr;
	MYSQL_STMT *stmt_delete = nullptr;
};

#endif // USE_REDIS
