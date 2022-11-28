#include "SRRdtReceiver.h"
#include "Global.h"

SRRdtReceiver::SRRdtReceiver():expectSequenceNumberRcvd(0)
{
	lastAckPkt.acknum = 127; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


SRRdtReceiver::~SRRdtReceiver()
{
}

bool SRRdtReceiver::inWindow(int num) {
	if (num+4 >= 128) {
		if (num > expectSequenceNumberRcvd || num < (expectSequenceNumberRcvd+4)%128) {
			return true;
		}
	} else {
		if (num < expectSequenceNumberRcvd+4 && num > expectSequenceNumberRcvd) {
			return true;
		}
	}
	return false;
}


void SRRdtReceiver::receive(const Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	printf("expectSequenceNumberRcvd:%d", this->expectSequenceNumberRcvd);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.checksum = 0;
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		printf("开始发送确认包\n");
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		printf("确认包发送成功\n");
		this->expectSequenceNumberRcvd = (1 + this->expectSequenceNumberRcvd) % 128; //接收序号在0-127之间切换
		while (window.size() > 0 && window.count(this->expectSequenceNumberRcvd) == 1) {
			//取出Message，向上递交给应用层
			printf("判断新的起始序列号是否已收到对应报文\n");
			Packet packet = window[expectSequenceNumberRcvd];
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			window.erase(expectSequenceNumberRcvd);

			lastAckPkt.checksum = 0;
			lastAckPkt.acknum = expectSequenceNumberRcvd; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
			this->expectSequenceNumberRcvd = (1 + this->expectSequenceNumberRcvd) % 128;
		}
		printf("发送完毕\n");
	} else if (checkSum == packet.checksum && inWindow(packet.seqnum)) {
		pUtils->printPacket("接收方正确收到发送方的报文,但不是正确顺序", packet);
		window.insert(pair<int, Packet>(packet.seqnum, packet));
		pUtils->printPacket("接收方发送最后正确接收对应的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
	} else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			printf("expectSequenceNumberRcvd:%d", this->expectSequenceNumberRcvd);
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
	if (this->window.size() > 0) {
		printf("开始输出接收方滑动窗口\n");
		printf("空");
		int i = expectSequenceNumberRcvd+1;
		if (i >= 128) {
			i %= 128;
		}
		for (i; inWindow(i); ) {
			if (window.count(i) == 1) {
				printf("%d", window[i].seqnum);
			} else {
				printf("空");
			}
			i++;
			if (i >= 128) {
				i %= 128;
			}
		}
	}
}