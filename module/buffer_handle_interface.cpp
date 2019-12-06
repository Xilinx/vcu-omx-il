#include "buffer_handle_interface.h"

BufferHandleInterface::~BufferHandleInterface() = default;
BufferHandleInterface::BufferHandleInterface() = default;
BufferHandleInterface::BufferHandleInterface(char* data, int size) : data{data}, size{size}
{
}
