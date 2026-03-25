#ifndef LIBUKMGE_GLOBAL_H
#define LIBUKMGE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBUKMGE_LIBRARY)
#  define LIBUKMGE_EXPORT Q_DECL_EXPORT
#else
#  define LIBUKMGE_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBUKMGE_GLOBAL_H
