#ifndef LIBBLINKYGLOBAL_H
#define LIBBLINKYGLOBAL_H

#ifndef QT_STATIC
#  if defined(BUILD_LIBBLINKY_LIB)
#    define LIBBLINKY_EXPORT Q_DECL_EXPORT
#  else
#    define LIBBLINKY_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define LIBBLINKY_EXPORT
#endif

#endif // LIBBLINKYGLOBAL_H
