﻿/*
 * MIT License
 *
 * Copyright (c) 2016 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include "Util/logger.h"
#include "Util/TimeTicker.h"
#include "Network/TcpServer.h"
#include "Network/TcpSession.h"

using namespace std;
using namespace toolkit;

class EchoSession: public TcpSession {
public:
	EchoSession(const Socket::Ptr &sock) :
			TcpSession(sock) {
		DebugL;
	}
	~EchoSession() {
		DebugL;
	}
	virtual void onRecv(const Buffer::Ptr &buf) override{
		//处理客户端发送过来的数据
		TraceL << buf->data();
		//把数据回显至客户端
        *(this) << "recved " << buf->size() << ": " << buf;
	}
	virtual void onError(const SockException &err) override{
		//客户端断开连接或其他原因导致该对象脱离TCPServer管理
		WarnL << err.what();
	}
	virtual void onManager() override{
		//定时管理该对象，譬如会话超时检查
		DebugL;
		if(_ticker.createdTime() > 5 * 1000){
			shutdown();
		}
	}

private:
	Ticker _ticker;
};


int main() {
	//退出程序事件处理
	signal(SIGINT, [](int){EventPollerPool::Instance().shutdown();});
	//初始化日志模块
	Logger::Instance().add(std::make_shared<ConsoleChannel>());
	Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

	TcpServer::Ptr server(new TcpServer(nullptr, nullptr));
	server->start<EchoSession>(9000);//监听9000端口

	EventPollerPool::Instance().wait();
	return 0;
}