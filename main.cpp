#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>


std::string csv("");

#include "ImageLib.h"

#include "ConvertX.h"
#include "ConvertY.h"
#include "HttpRequest.h"


using namespace std;
using namespace sf;


char datafragment[1024 * 1024 * 3];	// 3 МБ
char databyte[1024 * 1024 * 200];	// 150 МБ



char request_ok[] =
"HTTP/1.1 200 OK\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Content-Length: 0\r\n"
"Connection: keep-alive\r\n"
"Keep-Alive: timeout=60\r\n"
"Server: FastServer\r\n"
"Data: Grace_Kelly.ogg\r\n"
"Content-Type: text/plain; charset=utf-8\r\n"
"\r\n";

char request_not[] =
"HTTP/1.1 405 Method Not Allowed\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Allow: GET, POST, HEAD\r\n"
"Content-Length: 234\r\n"
"\r\n";

char request_error[] =
"HTTP/1.1 500 ERROR\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Content-Length: 0\r\n"
"\r\n";


bool fastParserMethod(string str)
{
	string::size_type n;
	//string::size_type m;
	//string::size_type c;

	// Выделение первой строки c методом
	n = str.find("\n");
	string line0 = str.substr(0, n);

	if (line0.find("OPTION") != string::npos) {
		return true;
	}
	else
		return false;
}

// Получаем все данные, а не < 64 КБ
size_t resendAllData(TcpSocket* _socket, size_t _size) {

	size_t shift = 0;
	size_t received = 0;
	sf::Socket::Status ready;

	int it = 0;
	while (true) {
		//ready = ;	// 2^16 = 65536 максимум за пакет
		if ((ready = _socket->receive(datafragment, sizeof(datafragment), received)) == Socket::Status::Done) {

			ofstream fout("data.txt", ios_base::app);
			for (int i = 0; i < received; i++) {
				
				databyte[shift + i] = datafragment[i];
				fout << datafragment[i]; // запись строки в файл
			}
			fout.close();

			shift += received;

			cout << "[" << it << "]: " << ready << endl;			// Вывод номера приема данных
			cout << "[received]: " << received << endl;				// Вывод размера полученнных данных
			cout << "[shift]: " << shift << endl;					// Вывод полученных данных за сессию


			string ctr(datafragment);
			if (fastParserMethod(ctr)) {
				cout << "[---OPTION---]: break" << endl;
				break;
			}
			// Очень тупо но времени нет
			if (datafragment[received - 4] == '-' && datafragment[received - 3] == '-') {
				cout << "[------]: break" << endl;
				break;
			}
				

			received = 0;
			it++;
			//sleep(milliseconds(3));
		}
		
	}

	return shift;
}

Color funcY_MusicToColor(Uint16 S);

int main(int argc, char* argv[])
{

	//setlocale(LC_ALL, "Russian");
	cout << "[Server start :9090]" << endl;


	//db_convert();

	/*Image img1;
	Image img2;

	img1.loadFromFile("1.png");
	img2.loadFromFile("2.png");*/

	

	//Color tmp = funcY_MusicToColor(51821);

	sf::TcpListener listener;
	if (listener.listen(9090) != sf::Socket::Done)
		cout << "[Error]: port 9090 blocked" << endl;


	int r = 0;

	while (true) {

		if (Keyboard::isKeyPressed(Keyboard::Escape))
			break;

		sf::TcpSocket socket;		// программный интерфейс для обеспечения обмена данными между процессами

		size_t received = 0;
		size_t shift = 0;
		size_t size = sizeof(databyte);

		// Ожидание данных от клиента
		cout << "[waiting connect]" << endl;
		if (listener.accept(socket) != sf::Socket::Done)// <- Блокировка. Ожидание подключения.
			cout << "[Fatal System Error]" << endl;
		else
			cout << "[Connect http client]: " << socket.getRemoteAddress() << endl;

		cout << "[connect]" << endl;
		shift = resendAllData(&socket, size);

		cout << "[get data]: " << shift << endl;
		/*ofstream fout("data_test[" + std::to_string(r) + "].txt");
		for (int i = 0; i < shift; i++) {
			fout << databyte[i]; // запись строки в файл
			//cout << databyte[i] << endl;
		}
		fout.close();*/
		r++;


		HttpRequest hr2(databyte, shift);
		// Если метод запроса поддерживается
		if (hr2.parserMethod()) {
			// И запос POST то получаем данные
			if (hr2.method == "POST") {
				hr2.parser();
				hr2.showHeaders();

				hr2.saveBody(hr2.http_body_name);	// Сохраняет фаил как есть

				if (hr2.type == 0)
					hr2.saveImagesFromMemory("image/" + hr2.http_body_name);
				else if (hr2.type == 1) {
					hr2.saveMusicFromMemory(hr2.http_body_name);
					// Отправка ответа
					received = 0;

					string ref = send_http_csv(csv);



					socket.send(ref.c_str(), ref.length(), received);
					cout << "[received]:" << received << endl;
					cout << "[data send]" << endl;
					socket.disconnect();
					continue;

				}
					

				// hr2.saveMusicFromMemory("audio//" + hr2.http_body_name);
			}
		}
		// Возврат ответа 21:07 11.11.17
		received = 0;

		string ref = send_http_csv(	"data.jpg;Grace_Kelly.ogg;0.64\r\n"
									"data.jpg;Elvis_Presley.ogg;0.74\r\n"
									"data.jpg;Elvis_Presley1.ogg;0.76\r\n");

		socket.send(ref.c_str(), ref.length(), received);
		cout << "[received]:" << received << endl;
		cout << "[data send]" << endl;
		socket.disconnect();

		//sleep(milliseconds(2500));
	}

	system("pause");

	return 0;
}