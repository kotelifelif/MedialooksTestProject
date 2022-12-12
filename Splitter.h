﻿#include "ISplitter.h"

#include <map>
#include <chrono>

struct Client {
	uint32_t m_unClientID {0};
	size_t m_zLattency {0};
	size_t m_zDropped {0};
	std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration>
		m_unTime;
};


class Splitter : public ISplitter {
public:
	Splitter(const size_t _zMaxBuffers, const size_t _zMaxClients);

	bool SplitterInfoGet(size_t* _pzMaxBuffers, size_t* _pzMaxClients) override;

	// Кладём данные в очередь. Если какой-то клиент не успел ещё забрать свои данные,
	// и количество буферов (задержка) для него больше максимального значения, то ждём
	// пока не освободятся буфера (клиент заберет данные) не более _nTimeOutMsec (**).
	// Если по истечению времени данные так и не забраны, то удаляем старые данные для
	// этого клиента, добавляем новые (по принципу FIFO) (*). Возвращаем код ошибки,
	// который дает понять что один или несколько клиентов “пропустили” свои данные.
	int32_t SplitterPut(const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, int32_t _nTimeOutMsec) override;

	// Сбрасываем все буфера, прерываем все ожидания. (после вызова допустима дальнейшая
	// работа)
	int32_t SplitterFlush() override;

	// Добавляем нового клиента - возвращаем уникальный идентификатор клиента.
	bool SplitterClientAdd(uint32_t* _punClientID) override;

	// Удаляем клиента по идентификатору, если клиент находиться в процессе ожидания
	// буфера, то прерываем ожидание.
	bool SplitterClientRemove(uint32_t _unClientID) override;

	// Перечисление клиентов, для каждого клиента возвращаем его идентификатор, количество
	// буферов в очереди (задержку) для этого клиента а также количество отброшенных буферов.
	bool SplitterClientGetCount(size_t* _pnCount) override;
	bool SplitterClientGetByIndex(size_t _zIndex, uint32_t* _punClientID, size_t* _pzLatency, size_t* _pzDropped) override;

	// По идентификатору клиента возвращаем задержку
	bool SplitterClientGetById(uint32_t _unClientID, size_t* _pzLatency, size_t* _pzDropped) override;

	// По идентификатору клиента запрашиваем данные, если данных пока нет, то ожидаем
	// не более _nTimeOutMsec (**) пока не будут добавлены новые данные, в случае превышения
	// времени ожидания - возвращаем ошибку.
	int32_t SplitterGet(uint32_t _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int32_t _nTimeOutMsec) override;

	// Закрытие объекта сплиттера - все ожидания должны быть прерваны все вызовы возвращают
	// соответствующую ошибку. Все клиенты удалены. (после вызова допустимо добавление
	// новых клиентов и дальнейшая работа)
	void SplitterClose() override;

private:
	bool FindFreeId(uint32_t* _punClientID);

	size_t m_zMaxBuffers;
	size_t m_zMaxClients;
	uint32_t m_unCurrentClientSize;
	std::map<uint32_t, Client> m_clients;
};