#include "Splitter.h"

bool Splitter::SplitterInfoGet(size_t* _pzMaxBuffers, size_t* _pzMaxClients) {
	return true;
}

// Кладём данные в очередь. Если какой-то клиент не успел ещё забрать свои данные, и количество буферов (задержка) для него больше максимального значения, то ждём пока не освободятся буфера (клиент заберет данные) не более _nTimeOutMsec (**). Если по истечению времени данные так и не забраны, то удаляем старые данные для этого клиента, добавляем новые (по принципу FIFO) (*). Возвращаем код ошибки, который дает понять что один или несколько клиентов “пропустили” свои данные.
int32_t Splitter::SplitterPut(const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, int32_t _nTimeOutMsec) {
	return int32_t(0);
}

// Сбрасываем все буфера, прерываем все ожидания. (после вызова допустима дальнейшая работа)
int32_t Splitter::SplitterFlush() {
	return int32_t(0);
}

// Добавляем нового клиента - возвращаем уникальный идентификатор клиента.
bool Splitter::SplitterClientAdd(uint32_t* _punClientID) {
	return true;
}

// Удаляем клиента по идентификатору, если клиент находиться в процессе ожидания буфера, то прерываем ожидание.
bool Splitter::SplitterClientRemove(uint32_t _unClientID) {
	return true;
}

// Перечисление клиентов, для каждого клиента возвращаем его идентификатор, количество буферов в очереди (задержку) для этого клиента а также количество отброшенных буферов.
bool Splitter::SplitterClientGetCount(size_t* _pnCount) {
	return true;
}

bool Splitter::SplitterClientGetByIndex(size_t _zIndex, uint32_t* _punClientID, size_t* _pzLatency, size_t* _pzDropped) {
	return true;
}

// По идентификатору клиента возвращаем задержку
bool Splitter::SplitterClientGetById(uint32_t _unClientID, size_t* _pzLatency, size_t* _pzDropped) {
	return true;
}

// По идентификатору клиента запрашиваем данные, если данных пока нет, то ожидаем не более _nTimeOutMsec (**) пока не будут добавлены новые данные, в случае превышения времени ожидания - возвращаем ошибку.
int32_t Splitter::SplitterGet(uint32_t _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int32_t _nTimeOutMsec) {
	return int32_t(0);
}

// Закрытие объекта сплиттера - все ожидания должны быть прерваны все вызовы возвращают соответствующую ошибку. Все клиенты удалены. (после вызова допустимо добавление новых клиентов и дальнейшая работа)
void Splitter::SplitterClose() {

}