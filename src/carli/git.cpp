#include "git.h"

#include <string>

int git_modified() {
  return GIT_MODIFIED;
}

std::string git_modified_string() {
  return GIT_MODIFIED_STR;
}

std::string git_revision_string() {
  return GIT_REVISION_STR;
}
