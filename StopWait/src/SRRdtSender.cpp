#include "SRRdtSender.h"
#include "Global.h"

SRRdtSender::SRRdtSender():expectSequenceNumberSend(0), window(), start(0)
{
}


SRRdtSender::~SRRdtSender()
{
}



bool SRRdtSender::getWaitingState() {
	return this->window.size() == 256;
}




bool SRRdtSender::send(const Message &message) {
    Packet packet{};
    packet.acknum = -1; //忽略该字段
	packet.seqnum = this->expectSequenceNumberSend;
	this->expectSequenceNumberSend = (1 + this->expectSequenceNumberSend) % 128;			//下一个发送序号在0-128之间切换
	packet.checksum = 0;
	memcpy(packet.payload, message.data, sizeof(message.data));
	packet.checksum = pUtils->calculateCheckSum(packet);
    if (window.size() == 0) {
        start = packet.seqnum;
		end = packet.seqnum;
    }
    window.insert(pair<int, Packet>(packet.seqnum, packet));
	if (this->window.size() <= 2) { //发送方处于等待确认状态
		end = packet.seqnum;
		pUtils->printPacket("发送方发送报文", packet);
		pns->startTimer(SENDER, Configuration::TIME_OUT, packet.seqnum);			//启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, packet);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	}
	if (window.size() > 0) {
		printf("开始输出窗口内的序列号\n");
		printf("%d", window[start].seqnum);
	}
	if (start != end) {
		printf("%d", window[end].seqnum);
	}
	
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {
	printf("收到确定包\n");
	if (window.size() > 0) {//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//如果校验和正确，并且确认序号=待确认的第一个正确帧序列号
		if (checkSum == ackPkt.checksum && ackPkt.acknum != -1) {
			int i = start;
			bool flag = false; //是否找到对应序列号
			for (int j = 0; j < 2; j++) {
				while (window.count(i) != 1) {
					printf("111\n");
					i++;
					if (i >= 128) {
						i %= 128;
					}
				}
				if (window[i].seqnum == ackPkt.acknum) { //找到确认包对应的报文
					printf("找到确认包对应的报文\n");
					window.erase(i);
					flag = true;
					if (window.size() >= 2) {
						int j = end+1;
						if (j >= 128) {
							j %= 128;
						}
						while (window.count(j) != 1) {
							printf("222\n");
							j++;
							if (j >= 128) {
								j %= 128;
							}
						}
						end = j;
						// 此时j为待加入窗口的报文的序列号
						pUtils->printPacket("发送方发送报文", window[j]);
						pns->startTimer(SENDER, Configuration::TIME_OUT, window[j].seqnum);			//启动发送方定时器
						pns->sendToNetworkLayer(RECEIVER, window[j]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
					}
					break;
				}
				i++;
				if (i >= 128) {
					i %= 128;
				}
			}
			if (flag == true && i != start) {
				printf("需要释放之前的序列号");
				for (int q = start; q != i;) {
					if (window.count(q) == 1) { //前面的序列号也应被释放
						window.erase(q);
						if (window.size() >= 2) {
							int j = end+1;
							if (j >= 128) {
								j %= 128;
							}
							while (window.count(j) != 1) {
								printf("333\n");
								j++;
								if (j >= 128) {
									j %= 128;
								}
							}
							end = j;
							// 此时j为待加入窗口的报文的序列号
							pUtils->printPacket("发送方发送报文", window[j]);
							pns->startTimer(SENDER, Configuration::TIME_OUT, window[j].seqnum);			//启动发送方定时器
							pns->sendToNetworkLayer(RECEIVER, window[j]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
						}
					}
					q++;
					if (q >= 128) {
						q %= 128;
					}
				}
			}
			if (flag == true && window.size() > 0) {
				
				printf("进入重置start模块");
				int j = start;
				while (window.count(j) != 1) {
					printf("111\n");
					j++;
					if (j >= 128) {
						j %= 128;
					}
				}
				start = j;
			}
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {
	int i = start;
	int num = 0;
	while (true) {
		num++;
		if (num == window.size()+1) {
			return;
		}
		printf("timeoutHandler\n");
		if (window.empty()) {
			printf("空空空！！！\n");
			return;
		}
		while (window.count(i) != 1) {
			printf("444\n");
			i++;
			if (i >= 128) {
				i %= 128;
			}
		}
		if (window[i].seqnum == seqNum) { // 找到超时的报文
			pUtils->printPacket("发送方定时器时间到，重发该报文", window[i]);
	        pns->stopTimer(SENDER, window[i].seqnum);										//首先关闭定时器
	        pns->startTimer(SENDER, Configuration::TIME_OUT, window[i].seqnum);			//重新启动发送方定时器
	        pns->sendToNetworkLayer(RECEIVER, window[i]);			//重新发送数据包
			return;
		}
		i++;
		if (i >= 128) {
			i %= 128;
		}
	}
}