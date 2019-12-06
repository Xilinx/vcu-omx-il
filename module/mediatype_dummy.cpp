#include "mediatype_dummy.h"

using namespace std;

DummyMediatype::DummyMediatype() = default;
DummyMediatype::~DummyMediatype() = default;

void DummyMediatype::Reset()
{
}

MediatypeInterface::ErrorType DummyMediatype::Get(string, void*) const
{
  return ErrorType::SUCCESS;
}

MediatypeInterface::ErrorType DummyMediatype::Set(string, void const*)
{
  return ErrorType::SUCCESS;
}

bool DummyMediatype::Check()
{
  return true;
}
