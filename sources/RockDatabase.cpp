// Copyright 2021 Tinkerrer

#include "RockDatabase.h"

#include <boost/log/trivial.hpp>
#include <iostream>

#include "../PicoSHA2/picosha2.h"

RockDatabase::RockDatabase(const std::string& database) { open(database); }

void RockDatabase::open(const std::string& database) {
  // Получаем вектор имен Column Families
  std::vector<std::string> column_families_names;
  DB::ListColumnFamilies(DBOptions(), database, &column_families_names);

  // Создаем вектор параметров открытия каждой Column Families
  std::vector<ColumnFamilyDescriptor> column_families;
  for (const auto& col_name : column_families_names) {
    column_families.push_back(
        ColumnFamilyDescriptor(col_name, ColumnFamilyOptions()));
  }

  // Открываем текущие DB
  Options options;
  options.create_if_missing = true;
  Status s = DB::Open(options, database, column_families, &handles, &db);

  // Выводим лог об ошибке

  printLog(s, "database open status");
}

void RockDatabase::print() {
  for (const auto& handle : handles) {
    Iterator* iterator = db->NewIterator(ReadOptions(), handle);
    for (iterator->SeekToFirst(); iterator->Valid(); iterator->Next()) {
      std::cout << iterator->key().ToString() << ": "
                << iterator->value().ToString() << std::endl;
    }
    delete iterator;
  }
}

void RockDatabase::close() {
  Status s;
  for (auto handle : handles) {
    delete handle;
  }
  s = db->Close();
  delete db;
  printLog(s, "database closing status");
}

void RockDatabase::copy_hashed(RockDatabase& db_to_paste) {
  Status s;

  for (auto handle : db_to_paste.handles) {
    s = db_to_paste.db->DropColumnFamily(handle);
    printLog(s, "dropping column family \"" + handle->GetName() + "\" status");
  }

  //Тут проблема с создание новых указателей handles
  // Убрать из коммита логи
  db_to_paste.handles.resize(handles.size());

  for (size_t i = 0; i < handles.size(); ++i) {
    ColumnFamilyDescriptor desc;
    handles[i]->GetDescriptor(&desc);
    s = db_to_paste.db->DB::CreateColumnFamily(desc.options, desc.name,
                                               &db_to_paste.handles[i]);
    printLog(s, "creating column family \"" + desc.name + "\" status");
  }

  for (size_t i = 0; i < handles.size(); ++i) {
    Iterator* base_iterator = db->NewIterator(ReadOptions(), handles[i]);
    for (base_iterator->SeekToFirst(); base_iterator->Valid();
         base_iterator->Next()) {
      // PicoSHA2 time
      std::string hash_hex_str;
      picosha2::hash256_hex_string(
          base_iterator->key().ToString() + base_iterator->value().ToString(),
          hash_hex_str);
      s = db_to_paste.db->Put(WriteOptions(), db_to_paste.handles[i],
                              base_iterator->key(), hash_hex_str);
      printLog(s, "adding element: \nkey: " + base_iterator->key().ToString() +
                      "\tvalue: " + hash_hex_str + "\nstatus");
    }
    delete base_iterator;
  }
}

RockDatabase::~RockDatabase() { close(); }

void RockDatabase::printLog(const Status& s, const std::string& message) {
  if (!s.ok()) {
    BOOST_LOG_TRIVIAL(error) << message << " : " << s.ToString();
    return;
  }
  BOOST_LOG_TRIVIAL(info) << message << " : " << s.ToString();
}