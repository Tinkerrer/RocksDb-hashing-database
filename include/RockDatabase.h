// Copyright 2021 Tinkerrer

#ifndef TEMPLATE_ROCKDATABASE_H
#define TEMPLATE_ROCKDATABASE_H

#include <rocksdb/db.h>
#include <rocksdb/iterator.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>

#include <string>

#include "../ThreadPool/ThreadPool.h"

using namespace ROCKSDB_NAMESPACE;

class RockDatabase {
 public:
  explicit RockDatabase(const std::string& database);

  void open(const std::string& database);

  void close();

  void copy_hashed(RockDatabase& db_to_paste);

  void print();

  ~RockDatabase();

 private:
  rocksdb::DB* db;

  std::vector<ColumnFamilyHandle*> handles;

  void printLog(const Status& s, const std::string& messeage);
};

#endif  // TEMPLATE_ROCKDATABASE_H
