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
std::vector<std::pair<std::string, std::string>> DataBase::getHistory(int room) {
	std::lock_guard<std::mutex>myLock(this->dbmtx);
	pqxx::connection connect(this->conn);
	pqxx::work tx(connect);
	std::vector<std::pair<std::string, std::string>>getm;
	pqxx::result res = tx.exec_params("SELECT users.log,history.message FROM history INNER JOIN users ON history.user_id=users.id WHERE history.room_id=$1 ORDER BY history.id DESC LIMIT 50", room);
	if (!res.empty()) {
		for (auto row = res.rbegin(); row != res.rend(); row++) {
			const auto& read = *row;//Разъименование,вектор не работал с обычным row.as<string>
			std::string nick = read[0].as<std::string>();
			std::string mes = read[1].as<std::string>();
			getm.push_back({ nick, mes });
		}
	}
		return getm;
	}
void DataBase::saveMsg(int nameid,const std::string& mes,int room_id) {
	pqxx::connection connect(this->conn);
	pqxx::work tx(connect);
	tx.exec_params("INSERT INTO history (user_id,message,room_id) VALUES($1,$2,$3) ", nameid, mes,room_id);//Сохранить 1 соо
	tx.commit();
}