#ifndef GO_BACK_N_RDT_RECEIVER_H
#define GO_BACK_N_RDT_RECEIVER_H
#include "RdtReceiver.h"

class GoBackNRdtReceiver : public RdtReceiver
{
private:
    int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	Packet lastAckPkt;				//上次发送的确认报文
public:
	GoBackNRdtReceiver();
	virtual ~GoBackNRdtReceiver();

public:
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用            
};

#endif