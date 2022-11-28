#ifndef SR_RDT_Sender_H
#define SR_RDT_Sender_H
#include "RdtSender.h"
#include <unordered_map>
#define SR_WINDOW_SIZE 4

class SRRdtSender : public RdtSender {
private:
    std::unordered_map<int, Packet> window;      // 滑动窗口
    int expectSequenceNumberSend;	// 下一个发送序号
    int start; // 窗口的开始报文序列号
	int end; // 窗口的结束报文序列号
public:
    SRRdtSender();
	virtual ~SRRdtSender();

	bool getWaitingState();
	bool send(const Message &message);					//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);					//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用
};

#endif