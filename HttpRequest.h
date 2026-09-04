#pragma once
#include<string>
#include<iostream>
enum class Status {
MSG,
CHANGE_NICK,
QUIT,
COMMAND,
COMPLETE,
AUTHORIZATION,
AUTH_LOG,
AUTH_PASS,
CURRENT_ROOM,
FIND_USER,
START_LS
};
class Pars {
private:
	Status current_stat;//Текущее состояние 
	//std::string key;//Временныая переменная дял ключа
	//std::string value;//временная переменная для значения
public:
	Pars() :current_stat(Status::COMMAND){}//По дефолту парсинг начинается с метода
	std::string message;
	std::string command;
	std::string log;
	std::string pass;
	std::string roomNum;
	
	//std::map<std::string, std::string>headers;
	void parse(const char* buffer, size_t size);//Принимает массив символов
	bool isComplete() const	{//Проверка на завершение парсинга
		return current_stat==Status::COMPLETE;
}
	void clean() {
		message.clear();
		command.clear();
		log.clear();
		pass.clear();
		current_stat = Status::COMMAND;
		roomNum.clear();
	}
	~Pars() {
		std::cout<<"[Parser] has been Deleted"<<std::endl;
}
};
