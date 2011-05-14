# Keep the whole psiidle in a subdir, because its header files are included
# like this: #include <psiidle/idle.h>, and we don't want to add the whole
# 3rdparty dir to the include path.
subdirs(psiidle)
