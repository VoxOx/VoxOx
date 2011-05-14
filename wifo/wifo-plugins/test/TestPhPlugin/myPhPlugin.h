#ifndef MY_TEST_PLUGIN_H
#define MY_TEST_PLUGIN_H

#include <phapi.h>
#include <owpl_plugin.h>

typedef int (*FileTransferReceivedCb_T)(const char * FileName, const char * From, int FileSize);
typedef int (*FileTransferProgressCb_T)(int TransferId, int Percent);

#endif