#ifndef GO_BACK_N_RDT_SENDER_H
#define GO_BACK_N_RDT_SENDER_H
#include "RdtSender.h"
#include <deque>
#define GO_BACK_N_WINDOW_SIZE 4
class GoBackNRdtSender : public RdtSender {
private:
    std::deque<Packet> window;      // 滑动窗口
    int expectSequenceNumberSend;	// 下一个发送序号

public:
	GoBackNRdtSender();
	virtual ~GoBackNRdtSender();

	bool getWaitingState();
	bool send(const Message &message);					//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);					//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用
};

#endif