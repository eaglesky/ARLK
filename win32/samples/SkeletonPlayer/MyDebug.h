#ifndef _MYDEBUG_H
#define _MYDEBUG_H

#include <iostream>
#ifdef _CONSOLEWIN
#define MYDEBUG(format, ...) printf(format, ## __VA_ARGS__)
#else
#define MYDEBUG(format, ...)
#endif


#endif