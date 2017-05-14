#pragma once

#include <unordered_map>
#include <iostream>
#include <string>

template <typename T>
class GraphNode {
  using NodeList = std::unordered_map<std::string, GraphNode*>;
  std::string guid;
  T _data;
  NodeList _links;

  public:
    GraphNode(std::string id, T data)
      : guid(id), _data(data) {};

    std::string id() {
      return guid;
    }
    T data() {
      return _data;
    }
    NodeList links() {
      return _links;
    }

    void link(GraphNode* node) {
      _links.insert({ node->id(), node });
    };

    GraphNode* find(std::string id) {
      auto item = _links.find(id);
      if (item == _links.end()) {
        return nullptr;
      }

      return item->second;
    };

    void print(std::ostream &os, std::string indent = "") {
      os << "GraphNode {" << std::endl;
      os << indent << "  guid: \"" << guid << "\"," << std::endl;
      os << indent << "  data: ";
      _data.print(os, indent + "  ");
      os << "," << std::endl;
      if (_links.size() == 0) {
        os << indent << "  links: []" << std::endl;
      } else {
        os << indent << "  links: [" << std::endl;
        for (auto item : _links) {
          auto node = item.second;
          os << indent << "    ";
          node->print_ref(os);
          os << "," << std::endl;
        }
        os << indent << "  ]"<< std::endl;
      }
      os << indent << "}";
    };

    void print_ref(std::ostream &os) {
      os << "\"" << guid << "\"";
    }
};
