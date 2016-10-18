/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#if USE_ROCKSDB

#include "database-rocksdb.h"

#include "log.h"
#include "filesys.h"
#include "exceptions.h"
#include "util/string.h"

#include "leveldb/db.h"


#define ENSURE_STATUS_OK(s) \
	if (!(s).ok()) { \
		throw DatabaseException(std::string("RocksDB error: ") + \
				(s).ToString()); \
	}


Database_RocksDB::Database_RocksDB(const std::string &savedir)
{
	rocksdb::Options options;
	options.OptimizeForSmallDb();
	options.create_if_missing = true;
	options.compression = rocksdb::kNoCompression; // MapBlocks are already compressed

	rocksdb::Status status = rocksdb::DB::Open(options,
		savedir + DIR_DELIM + "map.rocksdb", &m_database);
	ENSURE_STATUS_OK(status);
}

Database_RocksDB::~Database_RocksDB()
{
	delete m_database;
}

bool Database_RocksDB::saveBlock(const v3s16 &pos, const std::string &data)
{
	rocksdb::Status status = m_database->Put(rocksdb::WriteOptions(),
			i64tos(getBlockAsInteger(pos)), data);
	if (!status.ok()) {
		warningstream << "saveBlock: RocksDB error saving block "
			<< PP(pos) << ": " << status.ToString() << std::endl;
		return false;
	}

	return true;
}

void Database_RocksDB::loadBlock(const v3s16 &pos, std::string *block)
{
	std::string datastr;
	rocksdb::Status status = m_database->Get(rocksdb::ReadOptions(),
		i64tos(getBlockAsInteger(pos)), &datastr);

	*block = (status.ok()) ? datastr : "";
}

bool Database_RocksDB::deleteBlock(const v3s16 &pos)
{
	rocksdb::Status status = m_database->Delete(rocksdb::WriteOptions(),
			i64tos(getBlockAsInteger(pos)));
	if (!status.ok()) {
		warningstream << "deleteBlock: RocksDB error deleting block "
			<< PP(pos) << ": " << status.ToString() << std::endl;
		return false;
	}

	return true;
}

void Database_RocksDB::listAllLoadableBlocks(std::vector<v3s16> &dst)
{
	rocksdb::Iterator* it = m_database->NewIterator(rocksdb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		dst.push_back(getIntegerAsBlock(stoi64(it->key().ToString())));
	}
	ENSURE_STATUS_OK(it->status());  // Check for any errors found during the scan
	delete it;
}

#endif // USE_ROCKSDB

