/**
Copyright (c) 2020, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _MAIKEN_GRAPH_HPP_
#define _MAIKEN_GRAPH_HPP_

#include "maiken/app.hpp"

namespace maiken {

struct GraphItem {
  GraphItem(Application* _app) : app{_app} {}
  Application* app = nullptr;
  std::unordered_map<std::string, std::shared_ptr<GraphItem>> next;
};

class DepGrapher {
 public:
  DepGrapher() {}

  decltype(auto) build(Application& app) {
    root = std::make_shared<GraphItem>(&app);
    _build(root);

    while (items.size()) {
      _leafs(root);
      _prune();
    }

    std::reverse(deps.begin(), deps.end());
    return deps;
  }

 private:
  void _build(std::shared_ptr<GraphItem>& item) {
    for (auto a : item->app->dependencies()) {
      auto hash = a->hash();
      if (!items.count(hash)) {
        items.emplace(hash, std::make_shared<GraphItem>(a));
        _build(items.at(hash));
      }
      item->next[hash] = items.at(hash);
    }
  }

  void _erase(std::shared_ptr<GraphItem> item, std::shared_ptr<GraphItem> from) {
    for (auto& pair : from->next) _erase(item, pair.second);
    auto hash = item->app->hash();
    from->next.erase(hash);
  }

  void _erase(std::shared_ptr<GraphItem> item) {
    _erase(item, root);
    items.erase(item->app->hash());
  }

  void _prune() {
    for (auto& pair : leafs) {
      auto& it = pair.second;
      deps.emplace_back(it->app);
      _erase(it, root);
      items.erase(it->app->hash());
    };

    leafs.clear();
  }

  void _leafs(std::shared_ptr<GraphItem> item) {
    if (item->next.empty())
      leafs[item->app->hash()] = item;
    else
      for (auto& pair : item->next) _leafs(pair.second);
  }

  std::shared_ptr<GraphItem> root;
  std::vector<Application*> deps;
  std::unordered_map<std::string, std::shared_ptr<GraphItem>> items, leafs;
};

} /*namespace maiken*/

#endif /* _MAIKEN_GRAPH_HPP_ */
