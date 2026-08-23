#include"ChatServer.h"
#include <iostream>
#include<pqxx/pqxx>
int main() {
//setlocale(LC_ALL, "Russian");
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	int port;
	pqxx::connection connect("dbname=Chat_Server_DataBase user=postgres password=1234 host=localhost port=5432");
	DataBase myDB("dbname=Chat_Server_DataBase user=postgres password=1234 host=localhost port=5432");
	std::string table = "CREATE TABLE IF NOT EXISTS users(log VARCHAR(64),pass BYTEA)";
	std::string historyTable = "CREATE TABLE IF NOT EXISTS history(id SERIAL PRIMARY KEY,log VARCHAR(64),message TEXT)";
	pqxx::work trans(connect);
	trans.exec(table);
	trans.exec(historyTable);
	trans.commit();
	std::cout << "Enter your port" << std::endl;
	std::cin >> port;
	ChatServer myServer(port,myDB);
	if (!myServer.init()) {//Если возниклас ошибка с инициализицей
		std::cerr << "Ошибка инициализации,не удалось запустить сервер" << std::endl;
			system("Pause");
			return 1;
	}
	myServer.start();//Запуск бесконечного цикла
	system("Pause");
	WSACleanup();
	std::cout << "Сетевая бибилотека удалена из ОЗУ" << std::endl;
	system("Pause");
	return 0;
}