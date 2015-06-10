#ifndef FS_UTILS_H_INCLUDED
#define FS_UTILS_H_INCLUDED
namespace fs_utils
{
  void dirwalk(const char *,
            int (*)(const char * ),
            int (*)(const char * ),
            bool ignore_symlinks=true);

  class handler {
      public:
          virtual int handle_file(const char *)=0;
          virtual int handle_directory(const char *)=0;
          virtual ~handler(){};
          void dirwalk(const char * path, bool ignore_symlinks=true );
  };
}
#endif
