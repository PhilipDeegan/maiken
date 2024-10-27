

#include "maiken/github.hpp"
#include "mkn/kul/assert.hpp"

#include <cstdint>

namespace maiken {
bool static const premain = []() {
  github::URL = "localhost";
  github::port = 8000;
  return true;
}();

}  // namespace maiken

int test_repo() {
  std::string branch;
  bool b = maiken::Github<false>::GET_DEFAULT_BRANCH("owner", "repo", branch);
  mkn::kul::abort_if(!b);
  return !b;
}

int test_repo_fail() {
  std::string branch;
  bool b = maiken::Github<false>::GET_DEFAULT_BRANCH("owner", "repo_fail", branch);
  mkn::kul::abort_if(b);
  return b;
}

int test_repo_invalid_response() {
  std::string branch;
  bool b = maiken::Github<false>::GET_DEFAULT_BRANCH("owner", "repo_invalid_response", branch);
  mkn::kul::abort_if(b);
  return b;
}

int main(int argc, char* argv[]) {  //
  return test_repo() + test_repo_fail() + test_repo_invalid_response();
}
