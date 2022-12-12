#include <memory>
#include <vector>

class ISplitter {
public:
	virtual ~ISplitter() = default;

	virtual bool SplitterInfoGet(size_t* _pzMaxBuffers, size_t* _pzMaxClients) = 0;

	virtual int32_t SplitterPut(const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, int32_t _nTimeOutMsec) = 0;
	virtual int32_t SplitterFlush() = 0;
	virtual bool SplitterClientAdd(uint32_t* _punClientID) = 0;
	virtual bool SplitterClientRemove(uint32_t _unClientID) = 0;
	virtual bool SplitterClientGetCount(size_t* _pnCount) = 0;
	virtual bool SplitterClientGetByIndex(size_t _zIndex, uint32_t* _punClientID, size_t* _pzLatency, size_t* _pzDropped) = 0;
	virtual bool SplitterClientGetById(uint32_t _unClientID, size_t* _pzLatency, size_t* _pzDropped) = 0;
	virtual int32_t SplitterGet(uint32_t _nClientID, std::shared_ptr<std::vector<uint8_t>>& _pVecGet, int32_t _nTimeOutMsec) = 0;
	virtual void SplitterClose() = 0;
};
