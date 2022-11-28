#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include <unordered_map>

class SRRdtReceiver : public RdtReceiver {
private:
    std::unordered_map<int, Packet> window;      // 滑动窗口
    int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
    Packet lastAckPkt;				//上次发送的确认报文
public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
private:
    bool inWindow(int num); // 判断目标序列号是否在窗口内部
};

#endif