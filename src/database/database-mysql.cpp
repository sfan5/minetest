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

#include "config.h"

#if ENABLE_MYSQL

#include "database-mysql.h"

#include "settings.h"
#include "log.h"
#include "exceptions.h"
#include "util/string.h"

#include <mysql.h>

#define CHECKERR(s, m) \
	if (s) { \
		throw DatabaseException(std::string(m) + ": " + \
				mysql_error(connection)); \
	}

#define PREPARE_STMT(stmt, str) \
	CHECKERR(mysql_stmt_prepare(stmt, (str), strlen(str)) != 0, \
		"Failed to preprare statement")

Database_MySQL::Database_MySQL(Settings &conf)
{
	std::string user, password, database;
	try {
		user = conf.get("mysql_user");
		password = conf.get("mysql_password");
		database = conf.get("mysql_database");
	} catch (SettingNotFoundException &) {
		throw SettingNotFoundException("Set mysql_user, mysql_password and "
			"mysql_database in world.mt to use the MySQL backend");
	}
	std::string host = conf.exists("mysql_host") ? conf.get("mysql_host") : "localhost";
	u16 port = conf.exists("mysql_port") ? conf.getU16("mysql_port") : 3306;

	connection = mysql_init(NULL);
	if(connection == nullptr)
		throw DatabaseException("Failed to initialize mysql");

	if(!mysql_real_connect(connection, host.c_str(), user.c_str(),
			password.c_str(), database.c_str(), port, NULL, 0)) {
		std::string err = std::string("Connection error: ") + mysql_error(connection);
		mysql_close(connection);
		throw DatabaseException(err);
	}

	CHECKERR(mysql_query(connection,
		"CREATE TABLE IF NOT EXISTS `blocks` (\n"
		"x SMALLINT NOT NULL,\n"
		"y SMALLINT NOT NULL,\n"
		"z SMALLINT NOT NULL,\n"
		"data LONGBLOB NOT NULL,\n"
		"PRIMARY KEY (x, y, z)\n"
		")") != 0, "Failed to create table")
	CHECKERR(mysql_autocommit(connection, 0) != 0,
		"Failed to disable autocommit");
	initStatements();
}

Database_MySQL::~Database_MySQL()
{
	mysql_stmt_close(stmt_read);
	mysql_stmt_close(stmt_write);
	mysql_stmt_close(stmt_list);
	mysql_stmt_close(stmt_delete);

	mysql_close(connection);
}

void Database_MySQL::initStatements()
{
	CHECKERR((stmt_read = mysql_stmt_init(connection)) == nullptr,
		"Failed to create statement")
	CHECKERR((stmt_write = mysql_stmt_init(connection)) == nullptr,
		"Failed to create statement")
	CHECKERR((stmt_list = mysql_stmt_init(connection)) == nullptr,
		"Failed to create statement")
	CHECKERR((stmt_delete = mysql_stmt_init(connection)) == nullptr,
		"Failed to create statement")

	PREPARE_STMT(stmt_read,
		"SELECT data FROM `blocks` WHERE x = ? AND y = ? AND z = ?")
	PREPARE_STMT(stmt_write,
		"REPLACE INTO `blocks`(x, y, z, data) VALUES (?, ?, ?, ?)")
	PREPARE_STMT(stmt_list, "SELECT x, y, z FROM `blocks`")
	PREPARE_STMT(stmt_delete,
		"DELETE FROM `blocks` WHERE x = ? AND y = ? AND z = ?")
}

void Database_MySQL::beginSave()
{
}

void Database_MySQL::endSave()
{
	CHECKERR(mysql_commit(connection) != 0, "Failed to commit")
}

bool Database_MySQL::saveBlock(const v3s16 &pos, const std::string &data)
{
	MYSQL_BIND bind[4];
	memset(bind, 0, sizeof(bind));

	// bind x, y, z
	for(int i = 0; i < 3; i++) {
		bind[i].buffer_type = MYSQL_TYPE_SHORT;
		bind[i].is_null = static_cast<my_bool*>(0);
		bind[i].is_unsigned = 0;
	}
	bind[0].buffer = (void*) reinterpret_cast<const void*>(&pos.X);
	bind[1].buffer = (void*) reinterpret_cast<const void*>(&pos.Y);
	bind[2].buffer = (void*) reinterpret_cast<const void*>(&pos.Z);

	// bind data
	unsigned long length = data.size();
	bind[3].buffer_type = MYSQL_TYPE_BLOB;
	bind[3].buffer = (void*) reinterpret_cast<const void*>(data.c_str());
	bind[3].buffer_length = length;
	bind[3].length = &length;
	bind[3].is_null = static_cast<my_bool*>(0);

	CHECKERR(mysql_stmt_bind_param(stmt_write, bind) != 0,
		"Failed to bind parameters")

	if (mysql_stmt_execute(stmt_write) != 0) {
		warningstream << "saveBlock: Block failed to save "
			<< PP(pos) << ": " << mysql_stmt_error(stmt_write) << std::endl;
		return false;
	}

	mysql_stmt_reset(stmt_write);
	return true;
}

void Database_MySQL::loadBlock(const v3s16 &pos, std::string *block)
{
	MYSQL_BIND bind[3], result;
	memset(bind, 0, sizeof(bind));
	memset(&result, 0, sizeof(result));

	// bind x, y, z
	for(int i = 0; i < 3; i++) {
		bind[i].buffer_type = MYSQL_TYPE_SHORT;
		bind[i].is_null = static_cast<my_bool*>(0);
		bind[i].is_unsigned = 0;
	}
	bind[0].buffer = (void*) reinterpret_cast<const void*>(&pos.X);
	bind[1].buffer = (void*) reinterpret_cast<const void*>(&pos.Y);
	bind[2].buffer = (void*) reinterpret_cast<const void*>(&pos.Z);

	CHECKERR(mysql_stmt_bind_param(stmt_read, bind) != 0,
		"Failed to bind parameters")

	CHECKERR(mysql_stmt_execute(stmt_read) != 0, "Failed to execute statement")

	// bind result
	unsigned long length = 0;
	result.buffer = NULL;
	result.buffer_length = 0;
	result.length = &length;

	CHECKERR(mysql_stmt_bind_result(stmt_read, &result) != 0,
		"Failed to bind result")

	// probe the size of buffer we need
	int status = mysql_stmt_fetch(stmt_read);
	CHECKERR(status == 1, "Failed to fetch row")
	if (status == MYSQL_NO_DATA) {
		*block = "";
		mysql_stmt_reset(stmt_read);
		return;
	}

	// resize string and actually fetch data
	block->resize(length);
	result.buffer = reinterpret_cast<void*>( &(*block)[0] );
	result.buffer_length = length;
	CHECKERR(mysql_stmt_fetch_column(stmt_read, &result, 0, 0) != 0,
		"Failed to fetch column")

	mysql_stmt_reset(stmt_read);
}

bool Database_MySQL::deleteBlock(const v3s16 &pos)
{
	MYSQL_BIND bind[3];
	memset(bind, 0, sizeof(bind));

	// bind x, y, z
	for(int i = 0; i < 3; i++) {
		bind[i].buffer_type = MYSQL_TYPE_SHORT;
		bind[i].is_null = static_cast<my_bool*>(0);
		bind[i].is_unsigned = 0;
	}
	bind[0].buffer = (void*) reinterpret_cast<const void*>(&pos.X);
	bind[1].buffer = (void*) reinterpret_cast<const void*>(&pos.Y);
	bind[2].buffer = (void*) reinterpret_cast<const void*>(&pos.Z);

	CHECKERR(mysql_stmt_bind_param(stmt_delete, bind) != 0,
		"Failed to bind parameters")

	if (mysql_stmt_execute(stmt_delete) != 0) {
		warningstream << "saveBlock: Block failed to delete "
			<< PP(pos) << ": " << mysql_stmt_error(stmt_delete) << std::endl;
		return false;
	}

	mysql_stmt_reset(stmt_delete);
	return true;
}

void Database_MySQL::listAllLoadableBlocks(std::vector<v3s16> &dst)
{
	v3s16 pos;
	MYSQL_BIND result[3];
	memset(&result, 0, sizeof(result));

	CHECKERR(mysql_stmt_execute(stmt_list) != 0, "Failed to execute statement")

	// bind results
	unsigned long length = sizeof(s16);
	for(int i = 0; i < 3; i++) {
		result[i].buffer_length = length;
		result[i].length = &length;
	}
	result[0].buffer = reinterpret_cast<void*>(&pos.X);
	result[1].buffer = reinterpret_cast<void*>(&pos.Y);
	result[2].buffer = reinterpret_cast<void*>(&pos.Z);

	CHECKERR(mysql_stmt_bind_result(stmt_list, result) != 0,
		"Failed to bind result")

	while (true) {
		int status = mysql_stmt_fetch(stmt_list);
		CHECKERR(status == 1, "Failed to fetch row")
		if (status == MYSQL_NO_DATA)
			break;

		dst.push_back(pos);
	}

	mysql_stmt_reset(stmt_list);
}

#endif // USE_REDIS

