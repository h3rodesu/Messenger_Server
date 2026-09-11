#define NOMINMAX
#include "HttpMedia.h"
#include "httplib.h"//не в хидере тк библиотека тяжелая,cборка быстрее
#include <fstream>
#include<iostream>
#include<filesystem>
#include<ctime>
#include<string>

void MediaHttpServ::Start(int port) {
	httplib::Server svr;//ядро
	std::filesystem::create_directories("uploads");//проверка/создание цепочки папок
	svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {//вопрос и ответ
		std::string filename = "uploads/img_" + std::to_string(std::time(nullptr)) + ".jpg";
		std::ofstream file(filename, std::ios::binary);
		if (file.is_open()) {
			file.write(req.body.data(), req.body.size());//запись через указатель
			file.close();
			std::cout << "[HTTP] saved file " << filename << std::endl;
			res.status = 200;//т.е. все ок
			res.set_content(filename, "text/plain");
		}
		else {
			res.status = 500;//есть ошибка
			res.set_content("Failed to save file on server", "text/plain");
		}
		});
	svr.set_mount_point("/uploads", "./uploads");//чтение
	std::cout << "[HHTP] listening port " << port << std::endl;
	svr.listen("0.0.0.0", port);
}