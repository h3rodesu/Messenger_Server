#include"ChatServer.h"
#include <iostream>
#include<pqxx/pqxx>
#include "HttpMedia.h"
int main() {
//setlocale(LC_ALL, "Russian");	
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	std::thread mediaThread([]() {
		MediaHttpServ::Start(8081);
		});
	mediaThread.detach();
	std::cout << "HTTP server создан" << std::endl;

	pqxx::connection connect("dbname=Chat_Server_DataBase user=postgres password=1234 host=localhost port=5432");
	DataBase myDB("dbname=Chat_Server_DataBase user=postgres password=1234 host=localhost port=5432");
	std::string usersTable = "CREATE TABLE IF NOT EXISTS users(id SERIAL PRIMARY KEY,log VARCHAR(64),pass BYTEA)";
	std::string roomsTable = "CREATE TABLE IF NOT EXISTS room(id SERIAL PRIMARY KEY,room_name VARCHAR(64) NOT NULL)";
	std::string historyTable = "CREATE TABLE IF NOT EXISTS history(id SERIAL PRIMARY KEY,user_id INT REFERENCES users(id) ON DELETE CASCADE,message TEXT,room_id INT REFERENCES room(id) ON DELETE CASCADE)";
	std::string firstIndex = "CREATE INDEX IF NOT EXISTS idx_hist_room_id ON history(room_id)";
	pqxx::work trans(connect);
	trans.exec(usersTable);
	trans.exec(roomsTable);
	trans.exec(historyTable);
	trans.exec(firstIndex);
	trans.commit();
	ChatServer myServer(8080,myDB);
	if (!myServer.init()) {//Если возниклас ошибка с инициализицей
		std::cerr << "Ошибка инициализации,не удалось запустить сервер" << std::endl;
			system("Pause");
			return 1;
	}
	myServer.start();//Запуск бесконечного цикла
	
	WSACleanup();
	std::cout << "Сетевая бибилотека удалена из ОЗУ" << std::endl;

	return 0;
}