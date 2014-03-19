#!/bin/bash

DIR=$(echo "$0" | sed 's/\/[^\/]*$//')
cd "$DIR"

echo "#ifndef GIT_H" > git.h~ || exit 1

echo "#include <string>" >> git.h~ || exit 2

echo "#define GIT_MODIFIED \\" >> git.h~ || exit 3

STATUS=$(git status)
if [ $? -ne 0 ]; then exit 4; fi
echo "$STATUS" | grep -c modified >> git.h~

echo "#define GIT_REVISION \\" >> git.h~ || exit 5
git log -n 1 | head -n 1 | sed 's/commit //' >> git.h~ || exit 6
echo "#define GIT_STR_STR(x) #x" >> git.h~ || exit 7
echo "#define GIT_STR(x) GIT_STR_STR(x)" >> git.h~ || exit 8
echo "#define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)" >> git.h~ || exit 9
echo "#define GIT_REVISION_STR GIT_STR(GIT_REVISION)" >> git.h~ || exit 10

echo "int git_modified();" >> git.h~ || exit 11
echo "std::string git_modified_string();" >> git.h~ || exit 12
echo "std::string git_revision_string();" >> git.h~ || exit 13

echo "#endif" >> git.h~ || exit 14
(diff git.h git.h~ > /dev/null || cp git.h~ git.h) || exit 15
