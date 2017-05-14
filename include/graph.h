#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include <leveldb/db.h>
#include <leveldb/cache.h>

#include "graph_node.h"
#include "guid.h"

template <typename T>
class Graph {
  std::unordered_map<std::string, GraphNode<T>*> nodes;
  GuidGenerator guids;
  leveldb::DB* db;

  std::string tableName() {
    return typeid(T).name();
  }

  public:
    Graph(leveldb::DB* _db) : db(_db), guids(GuidGenerator()) {
      load();
    };
    ~Graph() {
      delete db;
    };

    void load() {
      std::cout << "loading..." << std::endl;
      auto it = db->NewIterator(leveldb::ReadOptions());
      for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto id = it->key().ToString();
        auto item = it->value().ToString();
        add(id, T::fromString(item));
      }
      std::cout << "loaded" << std::endl;
      delete it;
    }

    GraphNode<T>* add(std::string id, T data) {
      auto node = new GraphNode<T>(id, data);
      nodes.insert({ id, node });
      std::cout << "added: " << id << std::endl;
      return node;
    }

    GraphNode<T>* insert(T data) {
      std::stringstream id;
      id << guids.newGuid();
      auto guid = id.str();
      auto node = add(guid, data);

      auto str = data.toString();
      leveldb::WriteOptions options;
      db->Put(leveldb::WriteOptions(), guid, str);

      return node;
    };

    GraphNode<T>* find(std::string id) {
      auto item = nodes.find(id);
      if (item != nodes.end()) {
        return item->second;
      }

      leveldb::ReadOptions options;
      std::string result;
      auto s = db->Get(options, id, &result);
      if (!s.IsNotFound()) {
        result = s.ToString();
        return add(id, T::fromString(result));
      }

      return nullptr;
    };

    void print(std::ostream &os) {
      os << "Graph {" << std::endl;
      if (nodes.size() == 0) {
        os << "  children: []" << std::endl;
      } else {
        os << "  children: [" << std::endl;
        for (auto item : nodes) {
          auto node = item.second;
          os << "    ";
          node->print(os, "    ");
          os << "," << std::endl;
        }
        os << "  ]"<< std::endl;
      }
      os << "}";
    };
};
