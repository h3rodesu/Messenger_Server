#include "DataBaseManage.h"
#pragma warning(disable : 4996)//Чтоюы работал exec_params
//DataBase(const std::string data) :conn(data){}
bool DataBase::signin(const std::string& login, const std::string& password) {
	std::lock_guard<std::mutex>myLock(this->dbmtx);
	pqxx::connection connect(this->conn);//Отдельный коннект (если работа бует в разных потоках-для каждого потока свой коннект)
	pqxx::work tx(connect);
	//Нужно проверить логин и пароль 
	pqxx::result res = tx.exec_params("SELECT 1 FROM users WHERE log = $1 AND pass=digest($2,'sha256')", login, password);
	if (!res.empty()) {
		return true;
	}
	//тут клиент выведет что  неправильный логин или пароль
	return false;
}
bool DataBase::registration(const std::string& login, const std::string& password) {
	std::lock_guard<std::mutex>myLock(this->dbmtx);
	pqxx::connection connect(this->conn);
	pqxx::work tx(connect);
	pqxx::result logfind = tx.exec_params("SELECT 1 FROM users WHERE log =$1", login);//Поиск логина(юзер заходил)
	if (logfind.empty()) {//Если логин не найден
		tx.exec_params("INSERT INTO users(log,pass) VALUES($1,digest($2,'sha256'))", login, password);
		tx.commit();
		return true;
	}
		return false;
}
void DataBase::changelog(std::string oldnick, std::string newnick) {
	std::lock_guard<std::mutex>myLock(this->dbmtx);
	pqxx::connection connect(this->conn);
	pqxx::work tx(connect);
	tx.exec_params("UPDATE users SET log=$1 WHERE log=$2", newnick, oldnick);//позже заменить exec_params
	tx.commit();
}
std::vector<std::pair<std::string, std::string>> DataBase::getHistory() {	
	std::lock_guard<std::mutex>myLock(this->dbmtx);
	pqxx::connection connect(this->conn);
	pqxx::work tx(connect);
	std::vector<std::pair<std::string , std::string>>getm;
	pqxx::result res=tx.exec("SELECT log,message FROM history ORDER BY id DESC LIMIT 50");
	if (!res.empty()) {
		for (auto row = res.rbegin(); row != res.rend(); row++) {//от конца к начаул по тому что взяли из бд
			const auto& read = *row;//Разъименование,вектор не работал с обычным row.as<string>
			std::string nick = read[0].as<std::string>();
			std::string message = read[1].as<std::string>();
			getm.push_back({nick,message});
		}
	}
	return getm;
}
//void DataBase::saveHistory(std::vector<std::pair<std::string, std::string>>& history) {//перед этим заполнить вектор данными из деки
//	std::lock_guard<std::mutex>myLock(this->dbmtx);
//	pqxx::connection connect(this->conn);
//	pqxx::work tx(connect);
//	std::string del = "TRUNCATE TABLE history";
//	tx.exec(del);
//	for (auto iter= history.begin(); iter != history.end(); iter++) {
//		tx.exec_params("INSERT INTO history(log,message) VALUES ($1,$2)", iter->first, iter->second);
//	}
//	tx.commit();
//}
void DataBase::saveMsg(const std::string& name,const std::string& mes) {
	pqxx::connection connect(this->conn);
	pqxx::work tx(connect);
	tx.exec_params("INSERT INTO history (log,message) VALUES($1,$2) ", name, mes);//Сохранить 1 соо
	tx.commit();
}