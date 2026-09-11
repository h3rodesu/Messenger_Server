#pragma once
class MediaHttpServ {
public:
	static void Start(int port);//статик чтобы не было лишних объектов+передача в поток
private:
	MediaHttpServ() = default;//запрет на создание файлов
};