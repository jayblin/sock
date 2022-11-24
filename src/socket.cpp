#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

#include "windows_socket.cpp"

#else

#include "unix_socket.cpp"

#endif
