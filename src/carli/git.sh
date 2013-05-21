echo "#ifndef GIT_H" > git.h~
echo "#define GIT_MODIFIED \\\\" >> git.h~
git status | grep -c modified >> git.h~
echo "#define GIT_REVISION \\\\" >> git.h~
git log -n 1 | head -n 1 | sed 's/commit //' >> git.h~
echo "#define GIT_STR_STR(x) #x" >> git.h~
echo "#define GIT_STR(x) GIT_STR_STR(x)" >> git.h~
echo "#define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)" >> git.h~
echo "#define GIT_REVISION_STR GIT_STR(GIT_REVISION)" >> git.h~
echo "#endif" >> git.h~
(diff git.h git.h~ > /dev/null || cp git.h~ git.h)
