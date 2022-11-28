#include "GoBackNRdtSender.h"
#include "Global.h"


GoBackNRdtSender::GoBackNRdtSender():expectSequenceNumberSend(0), window()
{
}


GoBackNRdtSender::~GoBackNRdtSender()
{
}



bool GoBackNRdtSender::getWaitingState() {
	return this->window.size() == 256;
}




bool GoBackNRdtSender::send(const Message &message) {
    Packet packet{};
    packet.acknum = -1; //忽略该字段
	packet.seqnum = this->expectSequenceNumberSend;
	this->expectSequenceNumberSend = (1 + this->expectSequenceNumberSend) % 8;			//下一个发送序号在0-7之间切换
	packet.checksum = 0;
	memcpy(packet.payload, message.data, sizeof(message.data));
	packet.checksum = pUtils->calculateCheckSum(packet);
    window.push_back(packet);
	if (this->window.size() <= 4) { //发送方处于等待确认状态
		pUtils->printPacket("发送方发送报文", packet);
		pns->startTimer(SENDER, Configuration::TIME_OUT, packet.seqnum);			//启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, packet);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	}
	printf("打印滑动窗口内的报文序列号\n");
	for (int i = 0; i < 4 && i < window.size(); i++) {
		printf("%d\n", window[i].seqnum);
	}
	return true;
}

void GoBackNRdtSender::receive(const Packet &ackPkt) {
	if (window.size() > 0) {//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
		if (checkSum == ackPkt.checksum) {
			int flag = -1;
			for (int i = 0; i < window.size() && i < 4; i++) {
				if (window[i].seqnum == ackPkt.acknum) {
					flag = i;
					break;
				}
			}
			if (flag == -1) {
				return;
			}
			for (int i = 0; i <= flag; i++) {
				if (i == flag) {
					pUtils->printPacket("发送方正确收到确认", ackPkt);
				}
				pns->stopTimer(SENDER, window[0].seqnum);		//关闭定时器
            	window.pop_front();
				if (window.size() >= 4) {
					pUtils->printPacket("发送方发送报文", window[3]);
					pns->startTimer(SENDER, Configuration::TIME_OUT, window[3].seqnum);			//启动发送方定时器
					pns->sendToNetworkLayer(RECEIVER, window[3]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
				}
			}
		}
	}
	printf("打印滑动窗口内的报文序列号\n");
	for (int i = 0; i < 4 && i < window.size(); i++) {
		printf("%d\n", window[i].seqnum);
	}	
}

void GoBackNRdtSender::timeoutHandler(int seqNum) {
	bool flag = false;
    for (int i = 0; i < window.size() && i < 4; i++) {
        if (window[i].seqnum == seqNum) {
            flag = true;
        }
        if (flag == true) {
            pUtils->printPacket("发送方定时器时间到，重发从超时报文开始到窗口结束的报文", window[i]);
	        pns->stopTimer(SENDER, window[i].seqnum);										//首先关闭定时器
	        pns->startTimer(SENDER, Configuration::TIME_OUT, window[i].seqnum);			//重新启动发送方定时器
	        pns->sendToNetworkLayer(RECEIVER, window[i]);			//重新发送数据包
        }
    }

}