#include "utils.hpp"

NS::String *nsStr(const char *cStr, NS::StringEncoding encoding) {
  return NS::String::string(cStr, encoding);
}
