// MedialooksTestProject.cpp : Defines the entry point for the application.
//
#include "Splitter.h"

#include <iostream>
#include <memory>
#include <vector>
#include <cassert>
#include <thread>

using namespace std;

std::shared_ptr<ISplitter> SplitterCreate(size_t _zMaxBuffers, size_t _zMaxClients) {
	return make_shared<Splitter>(_zMaxBuffers, _zMaxClients);
}

void FillVector(const size_t size,  std::shared_ptr<std::vector<uint8_t>>& _pVec) {
	for (size_t i = 0; i < size; ++i) {
		_pVec->push_back(i);
	}
}

void MultithreadTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	const size_t vecSize = 1000;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	splitter->SplitterClientAdd(&id);
	splitter->SplitterClientAdd(&id);
	splitter->SplitterClientAdd(&id);
	thread t1([&]() {
		const size_t timeOutSec = 100;
		std::shared_ptr<std::vector<uint8_t>> vec;
		FillVector(1000, vec);
		splitter->SplitterPut(vec, timeOutSec);
		});
	thread t2([&]() {
		const size_t timeOutSec = 200;
		std::shared_ptr<std::vector<uint8_t>> vec;
		FillVector(1000, vec);
		splitter->SplitterPut(vec, timeOutSec);
		splitter->SplitterGet(1, vec, -1);
		});
	thread t3([&]() {
		const size_t timeOutSec = 300;
		std::shared_ptr<std::vector<uint8_t>> vec;
		FillVector(1000, vec);
		splitter->SplitterPut(vec, timeOutSec);
		});
	t1.join();
	t2.join();
	t3.join();
}

void AddClientTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	splitter->SplitterClientAdd(&id);
	assert(id == 1);
}

void AddMultipleClientTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	for (size_t i = 0; i < maxClients; ++i) {
		splitter->SplitterClientAdd(&id);
		assert(id == (i + 1));
	}
}

void RemoveClientTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	splitter->SplitterClientAdd(&id);
	splitter->SplitterClientRemove(id);
	size_t lattency;
	size_t dropped;
	splitter->SplitterClientGetById(id, &lattency, &dropped);
}

void GetClientByIdTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	splitter->SplitterClientAdd(&id);
	size_t lattency;
	size_t dropped;
	splitter->SplitterClientGetById(id, &lattency, &dropped);
	cout << "id - " << id << " lattency - " << lattency << " dropped - " << dropped << endl;
}

void GetClientByIndexTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	splitter->SplitterClientAdd(&id);
	splitter->SplitterClientAdd(&id);
	size_t lattency;
	size_t dropped;
	const size_t index = 1;
	splitter->SplitterClientGetById(id, &lattency, &dropped);
	cout << "index - " << index << " id - " << id << " lattency - " << lattency << " dropped - " << dropped << endl;
}

void PutGetTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	const size_t vecSize = 1000;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	splitter->SplitterClientAdd(&id);
	splitter->SplitterClientAdd(&id);
	std::shared_ptr<std::vector<uint8_t>> vec;
	const size_t timeOutMSec = 1000;
	FillVector(vecSize, vec);
	splitter->SplitterPut(vec, timeOutMSec);
	std::shared_ptr<std::vector<uint8_t>> getVec;
	splitter->SplitterGet(id, getVec, timeOutMSec);
	for (size_t i = 0; i < vecSize; ++i) {
		assert(i == getVec->at(i));
	}
	size_t count;
	splitter->SplitterClientGetCount(&count);
	assert(count == 2);
}

void GetInfoTest() {
	uint32_t id = 0;
	const size_t maxBuffers = 3;
	const size_t maxClients = 3;
	const size_t vecSize = 1000;
	auto splitter = SplitterCreate(maxBuffers, maxClients);
	size_t currentClients;
	size_t currentBuffers;
	splitter->SplitterInfoGet(&currentClients, &currentBuffers);
	assert(currentClients == maxBuffers);
	assert(currentBuffers == maxClients);
}

int main()
{
	MultithreadTest();
	AddClientTest();
	AddMultipleClientTest();
	RemoveClientTest();
	GetClientByIdTest();
	GetClientByIndexTest();
	PutGetTest();
	GetInfoTest();
	return 0;
}
