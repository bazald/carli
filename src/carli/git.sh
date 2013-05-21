echo "#ifndef GIT_H" > git.h~ || exit 1
echo "#define GIT_MODIFIED \\\\" >> git.h~ || exit 2
git status | grep -c modified >> git.h~ || exit 3
echo "#define GIT_REVISION \\\\" >> git.h~ || exit 4 
git log -n 1 | head -n 1 | sed 's/commit //' >> git.h~ || exit 5
echo "#define GIT_STR_STR(x) #x" >> git.h~ || exit 6
echo "#define GIT_STR(x) GIT_STR_STR(x)" >> git.h~ || exit 7
echo "#define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)" >> git.h~ || exit 8
echo "#define GIT_REVISION_STR GIT_STR(GIT_REVISION)" >> git.h~ || exit 9
echo "#endif" >> git.h~ || exit 10
(diff git.h git.h~ > /dev/null || cp git.h~ git.h) || exit 11
