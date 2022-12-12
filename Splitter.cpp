#include "Splitter.h"

Splitter::Splitter(const size_t _zMaxBuffers, const size_t _zMaxClients) : 
	m_zMaxBuffers(_zMaxBuffers), 
	m_zMaxClients(_zMaxClients),
	m_unCurrentClientSize(0)
{
}

bool Splitter::SplitterInfoGet(size_t* _pzMaxBuffers, size_t* _pzMaxClients) {
	*_pzMaxBuffers = m_zMaxBuffers;
	*_pzMaxClients = m_zMaxClients;
	return true;
}

// Кладём данные в очередь. Если какой-то клиент не успел ещё забрать свои данные,
// и количество буферов (задержка) для него больше максимального значения, то ждём
// пока не освободятся буфера (клиент заберет данные) не более _nTimeOutMsec (**).
// Если по истечению времени данные так и не забраны, то удаляем старые данные для
// этого клиента, добавляем новые (по принципу FIFO) (*). Возвращаем код ошибки,
// который дает понять что один или несколько клиентов “пропустили” свои данные.
int32_t Splitter::SplitterPut(const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, int32_t _nTimeOutMsec) {
	return int32_t(0);
}

// Сбрасываем все буфера, прерываем все ожидания. (после вызова допустима дальнейшая работа)
int32_t Splitter::SplitterFlush() {
	return int32_t(0);
}

// Добавляем нового клиента - возвращаем уникальный идентификатор клиента.
bool Splitter::SplitterClientAdd(uint32_t* _punClientID) {
	if ((m_clients.size() == m_zMaxClients) || !FindFreeId(_punClientID)) {
		return false;
	}
	Client client;
	client.m_unClientID = *_punClientID;
	client.m_unTime = std::chrono::system_clock::now();
	m_clients[*_punClientID] = client;
	return true;
}

// Удаляем клиента по идентификатору, если клиент находиться в процессе ожидания
// буфера, то прерываем ожидание.
bool Splitter::SplitterClientRemove(uint32_t _unClientID) {
	uint32_t unClientIDCopy = _unClientID;
	if (_unClientID == 0 || !FindFreeId(&unClientIDCopy)) {
		return false;
	}
	m_clients.erase(_unClientID);
	return true;
}

// Перечисление клиентов, для каждого клиента возвращаем его идентификатор, количество
// буферов в очереди (задержку) для этого клиента а также количество отброшенных буферов.
bool Splitter::SplitterClientGetCount(size_t* _pnCount) {
	return true;
}

bool Splitter::SplitterClientGetByIndex(size_t _zIndex, uint32_t* _punClientID, size_t* _pzLatency, size_t* _pzDropped) {
	if (_zIndex > m_zMaxClients) {
		return false;
	}
	auto clientBegin = m_clients.begin();
	for (size_t i = 0; i < _zIndex; ++i) {
		++clientBegin;
	}
	Client client = clientBegin->second;
	*_pzLatency = client.m_zLattency;
	*_pzDropped = client.m_zDropped;
	return true;
}

// По идентификатору клиента возвращаем задержку
bool Splitter::SplitterClientGetById(uint32_t _unClientID, size_t* _pzLatency, size_t* _pzDropped) {
	uint32_t unClientIDCopy = _unClientID;
	if (_unClientID == 0 || !FindFreeId(&unClientIDCopy)) {
		return false;
	}
	Client client = m_clients[_unClientID];
	*_pzLatency = client.m_zLattency;
	*_pzDropped = client.m_zDropped;
	return true;
}

// По идентификатору клиента запрашиваем данные, если данных пока нет, то ожидаем
// не более _nTimeOutMsec (**) пока не будут добавлены новые данные, в случае превышения
// времени ожидания - возвращаем ошибку.
int32_t Splitter::SplitterGet(uint32_t _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int32_t _nTimeOutMsec) {
	return int32_t(0);
}

// Закрытие объекта сплиттера - все ожидания должны быть прерваны все вызовы возвращают
// соответствующую ошибку. Все клиенты удалены. (после вызова допустимо добавление
// новых клиентов и дальнейшая работа)
void Splitter::SplitterClose() {

}

bool Splitter::FindFreeId(uint32_t* _punClientID)
{
	for (uint32_t i = 1; i <= UINT32_MAX; ++i) {
		if (m_clients.find(i) == m_clients.end()) {
			*_punClientID = i;
			return true;
		}
	}
	return false;
}
