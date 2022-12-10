#include <memory>
#include <vector>

class ISplitter {
public:
	virtual ~ISplitter() = default;

	virtual bool SplitterInfoGet(size_t* _pzMaxBuffers, size_t* _pzMaxClients) = 0;

	// Кладём данные в очередь. Если какой-то клиент не успел ещё забрать свои данные, и количество буферов (задержка) для него больше максимального значения, то ждём пока не освободятся буфера (клиент заберет данные) не более _nTimeOutMsec (**). Если по истечению времени данные так и не забраны, то удаляем старые данные для этого клиента, добавляем новые (по принципу FIFO) (*). Возвращаем код ошибки, который дает понять что один или несколько клиентов “пропустили” свои данные.
	virtual int32_t SplitterPut(const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, int32_t _nTimeOutMsec) = 0;

	// Сбрасываем все буфера, прерываем все ожидания. (после вызова допустима дальнейшая работа)
	virtual int32_t SplitterFlush() = 0;

	// Добавляем нового клиента - возвращаем уникальный идентификатор клиента.
	virtual bool SplitterClientAdd(uint32_t* _punClientID) = 0;

	// Удаляем клиента по идентификатору, если клиент находиться в процессе ожидания буфера, то прерываем ожидание.
	virtual bool SplitterClientRemove(uint32_t _unClientID) = 0;

	// Перечисление клиентов, для каждого клиента возвращаем его идентификатор, количество буферов в очереди (задержку) для этого клиента а также количество отброшенных буферов.
	virtual bool SplitterClientGetCount(size_t* _pnCount) = 0;
	virtual bool SplitterClientGetByIndex(size_t _zIndex, uint32_t* _punClientID, size_t* _pzLatency, size_t* _pzDropped) = 0;

	// По идентификатору клиента возвращаем задержку
	virtual bool SplitterClientGetById(uint32_t _unClientID, size_t* _pzLatency, size_t* _pzDropped) = 0;

	// По идентификатору клиента запрашиваем данные, если данных пока нет, то ожидаем не более _nTimeOutMsec (**) пока не будут добавлены новые данные, в случае превышения времени ожидания - возвращаем ошибку.
	virtual int32_t SplitterGet(uint32_t _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int32_t _nTimeOutMsec) = 0;

	// Закрытие объекта сплиттера - все ожидания должны быть прерваны все вызовы возвращают соответствующую ошибку. Все клиенты удалены. (после вызова допустимо добавление новых клиентов и дальнейшая работа)
	virtual void SplitterClose() = 0;
};
