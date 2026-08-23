#pragma once
#include<string>
#include<unordered_map>
#include<pqxx/pqxx>
#include<vector>
#include<mutex>
#include<iostream>
class DataBase {
private:
	//std::string name;
//std::string pass;
	//pqxx::connection& conn;
	std::string conn;//данные для подключения к бд
	std::mutex dbmtx;//Если 2 потока будут работать с данными пользователя
	
public:
	DataBase(const std::string con) :conn(con) {}
	//Хэш будет реализован самой бд
	//void getHash(const std::string& nickname, const std::string& password);//сделать хэш
	bool signin(const std::string& nickname, const std::string& password);//Вернет результат подключения
	//Мб метод для подключения сделаю
	bool registration(const std::string& login,const std::string&password);
	void connectToDB() {}
	std::vector<std::pair<std::string, std::string>> getHistory();//накопление сообщений
	void saveHistory(std::vector<std::pair<std::string, std::string>>& hystory);//взять сообщения из бд
	void changelog(std::string oldnick,std::string newnick);//для смены никнейма
	~DataBase(){}
};
