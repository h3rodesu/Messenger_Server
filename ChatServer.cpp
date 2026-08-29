#include "ChatServer.h"
#include "HttpRequest.h"

#include<memory>	
ChatServer::ChatServer(int s, DataBase& d) :mysocket(INVALID_SOCKET), myport(s), pool(4), db(d) {
	std::vector<std::pair<std::string,std::string>>loadData=db.getHistory();//получение архива сообщений 
	this->deq.assign(loadData.begin(),loadData.end());//сообщения закидываются в деку,обновлять потом вектор для каждого соо невыгодно
	std::cout << "Успешно загружен архив из " << this->deq.size() << " сообщений" << std::endl;
}//4 потока
bool ChatServer::init() {
	WSADATA wsa;//Тут адрес 
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);//Инициализация из сети
	if (result != 0) {//т.е. проблемы с запуском
		std::cerr << "WARNING! WINSOCK НЕ ЗАПУЩЕН" << std::endl;
		return false;//тк тип bool единица была бы равна тру
	}
	else {
		std::cout << "[WINSOCK] Успешно запущен" << std::endl;
	}
	//Дескриптор сокета
	SOCKET listensocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listensocket == INVALID_SOCKET) {
		std::cerr << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	mysocket = SafeSocket(listensocket);//Перенос сокета
//Заполнение структуры адреса и привязка сокета к порту 
	sockaddr_in serverADDR{};
	serverADDR.sin_family = AF_INET;//протокол IPv4
	serverADDR.sin_port = htons(myport);//Порт 
	serverADDR.sin_addr.s_addr = INADDR_ANY;//Слушает любой вхлдящий айпи адрес
	if(bind(mysocket.get(),(sockaddr*)&serverADDR,sizeof(serverADDR))==SOCKET_ERROR){//Проверка(если порт недоступен)
		std::cerr <<"[SERVER] ОШИБКА"<<myport<<" Занят другой программой " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	else {
		std::cout << "Привязка к порту " << myport << " прошла успешно" << std::endl;
	}
	if (listen(mysocket.get(), SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "[SERVER] ОШИБКА LISTEN" << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "[SERVER] ШЛЮЗ ОТКРЫТ" << std::endl;
	}
	return true;//bool тип требует вернуть значение
}
void ChatServer::start() {//Работа с клиентом
	while (true) {
		sockaddr_in clientAddr{};//{}чтобы не было системного мусора
		int clientAddrSize = sizeof(clientAddr);
		SOCKET ptimeclientSocket;//Голый сокет без дескриптора

		ptimeclientSocket = accept(mysocket.get(), (sockaddr*)&clientAddr, (&clientAddrSize));//Появление дескриптора
		if (ptimeclientSocket == INVALID_SOCKET|| ptimeclientSocket==0) {
			continue;//в случае ошибки возврат в начало while к новым подключеяим без ошибок
		}
		SafeSocket clientSocket(ptimeclientSocket);//Если ошибки выше нет то перенос сокета с дескриптором в объект класса SafeSocket чтобы работал принцип RAII
		std::cout << "Клиент на сервере" << std::endl;
		
		//ВРЕМЕННО ЗАКОММЕНТИРОВАННО	
		//auto unclientsocket = std::make_shared<SafeSocket>(std::move(clientSocket));//Закинули в указатель сам клиентский сокет
		//this->handlClient(unclientsocket);//чтобы вручную без пула вызвать метод
	 	
		auto unclientsocket = std::make_shared<SafeSocket>(std::move(clientSocket));//function отказывался принимать лямбду с некопируемым сокетом так что пришлось идти на крайние меры
		pool.add([this,unclientsocket=std::move(unclientsocket)]() mutable/*чтобы в лямбде не было const*/ noexcept{//Закинуть сокет клиента в очередь как задачу
			this->handlClient(std::move(unclientsocket));//достали сокет из указателя через разименование
			});
	
	
	
	
	}
}
void ChatServer::MessageBroadCast(const std::string& message, SOCKET sender) {
	std::lock_guard<std::mutex>myLock(this->mtx);
	for (const auto& read : this->Map) {
		SOCKET clientsock = read.first;
		if (clientsock == sender) {//чтобы не отправить сообщение отпрпаителю этого же сообщения
			continue;
		}
		send(clientsock, message.c_str(), (int)message.size(), 0);
	}
}

void ChatServer::handlClient(std::shared_ptr<SafeSocket>mySocket) {//Фоновый поток для чтения через метод recv
	char rxBuffer[1024];
	Pars parser;
	//std::string Welcome = "Welcome!\n Enter your nickname and password: ";
	//int flag = 1;
	//setsockopt(mySocket->get(), IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));//изза малого объёма строки без флага TCP_NODELAY она не отправлялась первому клиенту
	//send(mySocket->get(), Welcome.c_str(), (int)Welcome.size(), 0);
	std::string nick;
	try {
		while (true) {//Блок регистрации
			memset(rxBuffer, 0, sizeof(rxBuffer));
			int recBytes = recv(mySocket->get(), rxBuffer, sizeof(rxBuffer) - 1, 0);
			

			//Для дебага!!!
			std::cout << "Принято байт" << recBytes << std::endl;
			std::cout << "Содержимое буфера" << std::endl;
			//Для дебага!!!
			
			if (recBytes <= 0) {
				{
					std::lock_guard<std::mutex>myLock(this->mtx);
					this->Map.erase(mySocket->get());//удаление  пользователя из мапы
				}
				std::cerr << "Nickname trouble" << std::endl;
				return;
			}
			parser.parse(rxBuffer, size_t(recBytes));
			if (parser.isComplete()) {
				if (parser.command == "SIGNIN") {
					if (this->db.signin(parser.log, parser.pass)) {
						{
							std::lock_guard<std::mutex>myLock(this->mtx);
							this->Map[mySocket->get()] = parser.log;
						
						}
						std::string nicelog = "Auth_OK|Successful authorization.";
						send(mySocket->get(), nicelog.c_str(), (int)nicelog.size(), 0);
						nick = parser.log;
						for (const auto& it : this->deq) {
							std::string sendarchive = "" + it.first + "| " + it.second + "\n";
							send(mySocket->get(), sendarchive.c_str(), (int)sendarchive.size(), 0);
						}
						break;
					}
					else {
						std::string errorlog = "Incorrect login or password,try again\n";
						send(mySocket->get(), errorlog.c_str(), (int)errorlog.size(), 0);
						parser.clean();//очистка перед повторным использованием
					}
				}
				else if (parser.command == "REGISTRATION") {

					if (this->db.registration(parser.log, parser.pass)) {
						{
							std::lock_guard<std::mutex>myLock(this->mtx);
							this->Map[mySocket->get()] = parser.log;
						}
						std::string nicelog = "Register_OK|Successful registration.";
						send(mySocket->get(), nicelog.c_str(), (int)nicelog.size(), 0);
						nick = parser.log;
						std::string succreg ="Succesfull registration!";
						send(mySocket->get(), succreg.c_str(), (int)succreg.size(), 0);
						break;
					}
					std::string errorreg = "This login is used,try to use another login.";
					send(mySocket->get(), errorreg.c_str(), (int)errorreg.size(), 0);
					parser.clean();
				}
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Краш бд " << e.what() << std::endl;
	}
	std::string SysMsg = "System: Greet User " + nick + "  ,now he is in the chat! ";
	this->MessageBroadCast(SysMsg, mySocket->get());
	this->processClientMsg(mySocket, nick);
}
void ChatServer::processClientMsg(std::shared_ptr<SafeSocket>sock, std::string nick) {
	char buf[1024];
	Pars parser;
	while (true) {
	int rec=recv(sock->get(), buf, sizeof(buf) - 1, 0);
	if (rec <=0) {
		std::cerr<<"WARNING!" << WSAGetLastError() << std::endl;
		std::cout << "User " << nick << " left" << std::endl;
		std::unique_lock<std::mutex>myLock(this->mtx);
		this->Map.erase(sock->get());//Удаление сокета из мапы чтобы вышедшему пользователю не отправялиоись сообщения
		break;//Из бесконечного цикла
	}
	else {
		buf[rec] = '\0';
		std::cout<<nick<<": "<< buf << std::endl;
	}
	parser.clean();//Очстка строк
	parser.parse(buf, (size_t)rec);//парсим прилетевшую информацию
	if (parser.command == "MSG") {//если прилеетло сообщение
		std::string UserMsg = "" + nick + "| " + parser.message + "\n";//Передаем то что распарсил парсер в строку с сообщением и делаем перенос строки
		this->MessageBroadCast(UserMsg, sock->get());
		Archive(nick, parser.message);
	}
	else if (parser.command == "QUIT") {
		std::string quitmes=""+nick+" leave from chat, bye-bye!";
		this->MessageBroadCast(quitmes, sock->get());
		{
			std::unique_lock<std::mutex>myLock(this->mtx);
			this->Map.erase(sock->get());
		}
		break;
	}
	else if (parser.command == "CHANGE_NICK") {//а сам ник после | распарсится как сообщение
		std::string newNick = parser.message;
		if (!newNick.empty()) {
			std::string oldNick = nick;
			{
				std::unique_lock<std::mutex>myLock(this->mtx);
				this->Map[sock->get()] = newNick;//так и так изменения нужно занести в мапу
			}
				std::unique_lock<std::mutex>bdosnova(this->mtx);//находится под мьютексом в dbmanage 
				this->db.changelog(oldNick, newNick);
			nick = newNick;
			std::string nickmsg = "System: User " + oldNick + " change nickName to " + newNick + "\n";
			this->MessageBroadCast(nickmsg, sock->get());
		}
	}
	
	}
}
void ChatServer::Archive(const std::string& nick, const std::string& message) {
	std::lock_guard<std::mutex>myLock(this->historymtx);
		deq.push_back({ nick,message });
	if (deq.size() > 50) {
		deq.pop_front();
	}
}
ChatServer::~ChatServer() {
	std::vector<std::pair<std::string, std::string>>save;
	{
	std::lock_guard<std::mutex>lockdb(this->historymtx);
		save.assign(this->deq.begin(), this->deq.end());
	}
	db.saveHistory(save);//тут свой мьютекс
	std::cout << "[WINSOCK] Сетевая библиотека удалена " << std::endl;
}
