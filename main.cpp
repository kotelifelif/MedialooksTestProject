// MedialooksTestProject.cpp : Defines the entry point for the application.
//
#include "Splitter.h"

#include <iostream>
#include <memory>

using namespace std;

std::shared_ptr<ISplitter> SplitterCreate(size_t _zMaxBuffers, size_t _zMaxClients) {
	return make_shared<Splitter>(_zMaxBuffers, _zMaxClients);
}

int main()
{
	auto splitter = SplitterCreate(3, 3);
	uint32_t unClientId;
	splitter->SplitterClientAdd(&unClientId);
	return 0;
}
