#include "Splitter.h"

#include <algorithm>

Splitter::Splitter(const size_t _zMaxBuffers, const size_t _zMaxClients) : 
	m_zMaxBuffers(_zMaxBuffers), 
	m_zMaxClients(_zMaxClients),
	m_bInterrupt(false)
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
	if (m_bInterrupt)
		return static_cast<int32_t>(SplitterResult::Interrupted);
	Keeper keeper(m_nThreadCount, m_cvVariableClear);

	time_point remove;
	SplitterResult result = SplitterResult::Success;
	if (m_zMaxBuffers == m_dData.size()) 
	{
		auto conditionWakeUp = [this, &remove] // checks if the slowest client got the first buffer element
		{
			if (m_bInterrupt)
				return true;
			remove = (m_pcOld ? m_pcOld->m_tpTime : std::chrono::system_clock::now());
			bool b = remove > m_dData.front().first;
			return b;
		};

		bool updateDropped = false;
		std::unique_lock<std::mutex> lockClient(m_mClientMutex);
		if (_nTimeOutMsec < 0)
		{
			m_cvVariablePut.wait(lockClient, conditionWakeUp);
			if (m_bInterrupt)
				return static_cast<int32_t>(SplitterResult::Interrupted);
		}
		else
		{
			m_cvVariablePut.wait_for(lockClient, std::chrono::milliseconds(_nTimeOutMsec), conditionWakeUp);
			if (m_bInterrupt)
				return static_cast<int32_t>(SplitterResult::Interrupted);
			updateDropped = true;
		}

		if (updateDropped)
		{
			Client* client = m_pcOld;
			time_point timeDrop;
			if (remove < m_dData.front().first) {
				timeDrop = m_dData.front().first;
			}
			else {
				timeDrop = remove;
			}
			while (client && client->m_tpTime < timeDrop)
			{
				++client->m_zDropped;
				result = SplitterResult::Dropped;
				client = client->m_pcNext;
			}
		}
	}

	// add data to the buffer
	{
		std::lock_guard<std::mutex> lockBuffer(m_mBufferMutex);
		if (m_zMaxBuffers == m_dData.size())
		{
			m_dData.pop_front(); // at least one pop
			while (!m_dData.empty() && remove > m_dData.front().first)
				m_dData.pop_front();
		}
		m_dData.emplace_back(std::chrono::system_clock::now(), _pVecPut);
	}
	m_cvVariableGet.notify_all();
	return static_cast<int32_t>(SplitterResult(result));
}

// Сбрасываем все буфера, прерываем все ожидания. (после вызова допустима дальнейшая работа)
int32_t Splitter::SplitterFlush() {
	return Flush(false);
}

// Добавляем нового клиента - возвращаем уникальный идентификатор клиента.
bool Splitter::SplitterClientAdd(uint32_t* _punClientID) {
	std::lock_guard<std::mutex> lock_guard(m_mClientMutex);
	if ((m_umClients.size() == m_zMaxClients) || !FindFreeId(_punClientID)) {
		return false;
	}
	Client client;
	client.m_unClientID = *_punClientID;
	client.m_tpTime = std::chrono::system_clock::now();
	m_umClients[*_punClientID] = client;
	return true;
}

// Удаляем клиента по идентификатору, если клиент находиться в процессе ожидания
// буфера, то прерываем ожидание.
bool Splitter::SplitterClientRemove(uint32_t _unClientID) {
	std::lock_guard<std::mutex> lock_guard(m_mClientMutex);
	uint32_t unClientIDCopy = _unClientID;
	if (_unClientID == 0 || !FindFreeId(&unClientIDCopy)) {
		return false;
	}
	m_umClients.erase(_unClientID);
	return true;
}

// Перечисление клиентов, для каждого клиента возвращаем его идентификатор, количество
// буферов в очереди (задержку) для этого клиента а также количество отброшенных буферов.
bool Splitter::SplitterClientGetCount(size_t* _pnCount) {
	std::lock_guard<std::mutex> lock_guard(m_mClientMutex);
	*_pnCount = m_umClients.size();
	return true;
}

bool Splitter::SplitterClientGetByIndex(size_t _zIndex, uint32_t* _punClientID, size_t* _pzLatency, size_t* _pzDropped) {
	std::lock_guard<std::mutex> lock_guard(m_mClientMutex);
	if (_zIndex > m_zMaxClients) {
		return false;
	}
	auto clientBegin = m_umClients.begin();
	for (size_t i = 0; i < _zIndex; ++i) {
		++clientBegin;
	}
	Client client = clientBegin->second;
	*_pzLatency = (std::chrono::system_clock::now() - client.m_tpTime).count();
	*_pzDropped = client.m_zDropped;
	return true;
}

// По идентификатору клиента возвращаем задержку
bool Splitter::SplitterClientGetById(uint32_t _unClientID, size_t* _pzLatency, size_t* _pzDropped) {
	std::lock_guard<std::mutex> lock_guard(m_mClientMutex);
	auto findIt = m_umClients.find(_unClientID);
	if (_unClientID == 0 || findIt == m_umClients.end()) {
		return false;
	}
	Client client = m_umClients[_unClientID];
	*_pzLatency = (std::chrono::system_clock::now() - client.m_tpTime).count();
	*_pzDropped = client.m_zDropped;
	return true;
}

// По идентификатору клиента запрашиваем данные, если данных пока нет, то ожидаем
// не более _nTimeOutMsec (**) пока не будут добавлены новые данные, в случае превышения
// времени ожидания - возвращаем ошибку.
int32_t Splitter::SplitterGet(uint32_t _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int32_t _nTimeOutMsec) {
	if (m_bInterrupt)
		return static_cast<int32_t>(SplitterResult::Interrupted);
	Keeper keeper(m_nThreadCount, m_cvVariableClear);

	time_point tpClient;
	{   // get the client time
		size_t ind;
		std::lock_guard<std::mutex> lock(m_mClientMutex);
		if (m_umClients.find(_nClientID) == m_umClients.end())
			return static_cast<int32_t>(SplitterResult::NoClient);
		tpClient = m_umClients[ind].m_tpTime;
	}

	// waiting
	{
		std::unique_lock<std::mutex> lock(m_mBufferMutex);
		auto conditionWakeUp = [this, tpClient]
		{
			return m_bInterrupt || (!m_dData.empty() && tpClient < m_dData.back().first);
		};
		if (_nTimeOutMsec < 0)
		{
			m_cvVariableGet.wait(lock, conditionWakeUp);
			if (m_bInterrupt)
				return static_cast<int32_t>(SplitterResult::Interrupted);
		}
		else
		{
			m_cvVariableGet.wait_for(lock, std::chrono::milliseconds(_nTimeOutMsec), conditionWakeUp);
			if (m_bInterrupt)
				return static_cast<int32_t>(SplitterResult::Interrupted);
			if (!(!m_dData.empty() && tpClient < m_dData.back().first))
				return static_cast<int32_t>(SplitterResult::Timeout);
		}

		// fill _pVecGet
		auto itBegin = std::upper_bound(m_dData.begin(), m_dData.end(), tpClient, [](time_point tp, const std::pair<time_point, std::shared_ptr<std::vector<uint8_t>>>& el) { return tp < el.first; });
		for (auto it = itBegin; it != m_dData.end(); ++it)
		{
			// assure it->first > tpClient
			_pVecGet->insert(_pVecGet->end(), it->second->begin(), it->second->end());
		}
	}

	// update the client
	{   // get client time
		size_t ind;
		std::lock_guard<std::mutex> lock(m_mClientMutex);
		if (m_umClients.find(_nClientID) == m_umClients.end())
			return static_cast<int32_t>(SplitterResult::NoClient);
		Client client = m_umClients[ind];
		client.m_tpTime = std::chrono::system_clock::now();
		if (*m_pcRecent != client)
		{
			ExtractClient(&client);
			PutClient(&client);
		}
	}
	m_cvVariablePut.notify_all();
	return static_cast<int32_t>(SplitterResult::Success);
}

// Закрытие объекта сплиттера - все ожидания должны быть прерваны все вызовы возвращают
// соответствующую ошибку. Все клиенты удалены. (после вызова допустимо добавление
// новых клиентов и дальнейшая работа)
void Splitter::SplitterClose() {
	Flush(true);
}

bool Splitter::FindFreeId(uint32_t* _punClientID) {
	for (uint32_t i = 1; i <= UINT32_MAX; ++i) {
		if (m_umClients.find(i) == m_umClients.end()) {
			*_punClientID = i;
			return true;
		}
	}
	*_punClientID = 0;
	return false;
}

int32_t Splitter::Flush(bool _bClear)
{
	m_bInterrupt = true;
	m_cvVariablePut.notify_all();
	m_cvVariableGet.notify_all();

	SplitterResult result = SplitterResult::Success;
	{
		std::unique_lock<std::mutex> lock(m_mBufferMutex);
		m_cvVariableClear.wait(lock, [this, &result]()
			{
				if (0 == m_nThreadCount)
					return true;
				else
				{
					result = SplitterResult::Interrupted;
					return false;
				}
			});
		if (_bClear)
			m_dData.clear();
	}

	if (_bClear)
	{
		std::lock_guard<std::mutex> lock(m_mClientMutex);
		m_umClients.clear();
	}
	m_bInterrupt = false;
	return static_cast<int32_t>(result);
}

void Splitter::ExtractClient(Client* client)
{
	if (m_pcOld == m_pcRecent)
	{
		m_pcOld = nullptr;
		m_pcRecent = nullptr;
	}
	else if (m_pcOld == client)
	{
		m_pcOld = client->m_pcNext;
		m_pcOld->m_pcPrev = nullptr;
	}
	else if (m_pcRecent == client)
	{
		m_pcRecent = client->m_pcPrev;
		m_pcRecent->m_pcNext = nullptr;
	}
	else
	{
		Client* p = client->m_pcPrev;
		Client* n = client->m_pcNext;
		p->m_pcNext = n;
		n->m_pcPrev = p;
	}
	client->m_pcPrev = nullptr;
	client->m_pcNext = nullptr;
}
void Splitter::PutClient(Client* client)
{
	if (m_pcRecent)
	{
		m_pcRecent->m_pcNext = client;
		client->m_pcPrev = m_pcRecent;
		m_pcRecent = client;
	}
	else
	{
		m_pcRecent = client;
		m_pcOld = client;
	}
}