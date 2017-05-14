#include <leveldb/db.h>
#include <leveldb/cache.h>
#include <msgpack.hpp>
#include <uv.h>

#include "graph.h"

struct Person {
  std::string first;
  std::string last;
  std::string email;
  MSGPACK_DEFINE(first, last, email);

  void print(std::ostream &os, std::string indent) {
    os << "Person {" << std::endl;
    os << indent << "  first: \"" << first << "\"," << std::endl;
    os << indent << "  last: \"" << last << "\"," << std::endl;
    os << indent << "  email: \"" << email << "\"" << std::endl;
    os << indent << "}";
  };

  std::string toString() {
    std::stringstream ss;
    msgpack::pack(ss, *this);
    return ss.str();
  };

  static Person fromString(std::string str) {
    std::istringstream ss(str);
    auto oh = msgpack::unpack(ss.str().data(), ss.str().size());
    return oh.get().as<Person>();
  };
};

namespace msgpack {
  MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
      template<>
      struct as<Person> {
          Person operator()(msgpack::object const& o) const {
            if (o.type != msgpack::type::ARRAY) {
              throw msgpack::type_error();
            }

            if (o.via.array.size != 1) {
              throw msgpack::type_error();
            }

            return Person {
              o.via.array.ptr[0].as<std::string>(),
              o.via.array.ptr[1].as<std::string>(),
              o.via.array.ptr[2].as<std::string>()
            };
          }
      };
    } // namespace adaptor
  } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

struct Container {
  int number;

  void print(std::ostream &os, std::string indent) {
    os << "Container {" << std::endl;
    os << indent << "  number: " << number << "," << std::endl;
    os << indent << "}";
  };
};

leveldb::DB* openDb(std::string name) {
  // db path
  char buffer[256];
  size_t len = sizeof(buffer);
  uv_cwd(buffer, &len);
  std::string path = std::string(buffer, len) + "/" + name;

  // db options
  leveldb::Options options;
  options.create_if_missing = true;
  options.block_cache = leveldb::NewLRUCache(100 * 1048576);

  // db instance
  leveldb::DB* db;
  auto status = leveldb::DB::Open(options, path, &db);
  if (!status.ok()) {
    throw std::runtime_error(status.ToString());
  }

  return db;
}

/**
 * Naive bread-first search
 *
 * WARNING: This does not break free of circular references!
 */
using PersonNode = GraphNode<Person>*;
PersonNode breadthSearch(PersonNode from, std::string id) {
  for (auto item : from->links()) {
    if (item.second->id() == id) {
      return item.second;
    }
  }
  for (auto item : from->links()) {
    auto n = breadthSearch(item.second, id);
    if (n != nullptr) {
      return n;
    }
  }

  return nullptr;
}

int main() {
  auto db = openDb("graphdb");
  auto graph = new Graph<Person>(db);
  auto me = graph->insert(Person {
    "stephen",
    "belanger",
    "admin@stephenbelanger.com"
  });
  auto you = graph->insert(Person {
    "other",
    "person",
    "me@example.com"
  });
  auto them = graph->insert(Person {
    "another",
    "person",
    "me@example.com"
  });
  me->link(you);
  you->link(them);

  auto node = breadthSearch(me, them->id());
  if (node != nullptr) {
    std::cout << "found: " << node->id() << std::endl;
  } else {
    std::cout << "could not find: " << them->id() << std::endl;
  }

  // graph->print(std::cout);

  delete graph;

  return 0;
}
