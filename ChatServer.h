#pragma once
#include "SafeSocket.h"
#include<thread>
#include<mutex>
#include<deque>
#include <iostream>
#include <condition_variable>
#include<map>
#include<string>
#include"ThreadPool.h"
#include "HttpRequest.h"
#include "DataBaseManage.h"
class ChatServer {
private:
	SafeSocket mysocket;
	std::mutex mtx;
	std::mutex historymtx;
	std::map<SOCKET,std::string> Map;
	int myport;//Порт-просто число
	ThreadPool pool;
	void processClientMsg(std::shared_ptr<SafeSocket>sockm, std::string nick);
	DataBase& db;
	std::deque<std::pair<std::string, std::string>>deq;
public:
	ChatServer(int port, DataBase& d);
	bool init();//Инициализация winsock,bind и listen
void start();//цикл для ацептов,главный поток-хостес,получил клиентский сокет-завернул в лямбду и передал в фоновый поток,ТОЛЬКО ОЖИДАНЕИ И ПРИЕМ КЛИЕНТОВ
	void handlClient(std::shared_ptr<SafeSocket>mySocket);//ПРиём сокетов клиентов через мув чтобы закинуть в поток пула,фоновй поток внутри пула
	void MessageBroadCast(const std::string& message, SOCKET sender);//Метод для рассылки
	void Archive(const std::string& login, const std::string& msg);
	~ChatServer();
};