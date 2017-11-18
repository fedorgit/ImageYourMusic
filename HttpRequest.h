#pragma once
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <fstream>

#include "ConvertX.h"

using namespace std;
using namespace sf;


Image MusicToImage(SoundBuffer &buffer);
SoundBuffer ImageToMusic(Image &image);

Image MusicToImageY(SoundBuffer &buffer);

class HttpRequest
{
	string http_request;	// Данные от клиента
	string http_headers;	// Заголовки запроса
	string http_body;		// Тело запроса
	size_t http_size;		// Размер http запроса в байтах 
public:
	string method;			// Методы: {GET, POST, OPTIONS}
private:
	string content_length;	// Размер тела запроса
	size_t content_size = -1;	// Размер тела запроса
	string content_type;	// Content-Type [example]:Content-Type:multipart/form-data; boundary=----WebKitFormBoundaryGURBsmZPmneAAiQ3
	string meta;			// Тип данных
	string boundary;		// Разделитель входящих данных

	// 10.11.17
	map <string, string> headers;

	string http_header_image;	// Заголовки изображения. Example:
								// Content-Disposition: form-data; name="AttachedFile1"; filename="horror-photo-1.jpg"
								// Content-Type: image/jpeg
public:
	string http_body_name;			// Выделенное название файла
	string http_body_content_type;	// Расширение файла (пока что определяем по формату) {Изоражение(jpg, png), Аудио(ogg, wav, flac)} 
	int type = -1;					// Тип данных (пока что определяем по формату) {Изоражение(jpg, png), Аудио(ogg, wav, flac)} 
private:
	Uint8* body;				// Тут чутка ошибка если что-то не будет работать то ошибка тут
	size_t body_len = -1;		// Да

public:
	HttpRequest(char* resend_http, size_t len);
	bool parserMethod();
	bool parser();
	bool parserHeaders();
	bool parserHeadersBody();	// Выделяет подзаголовки body {Image, Music}
	void showHeaders();
	void saveBody(string fileName);
	void saveImagesFromMemory(string fileName = "img2.png");
	void saveMusicFromMemory(string fileName = "music2.wav");

	~HttpRequest();
};

void replace_all(string str, const char *_PtrSrc, const char *_PtrDmt) {
	string::size_type sPos;
	while ((sPos = str.find(_PtrSrc, 0)) != string::npos)
		str.replace(sPos, 1, _PtrDmt);
}


string send_http_csv(string content) {

	char request_ok1[] =
		"HTTP/1.1 200 OK\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Content-Length: ";
		
	char request_ok2[] = "\r\n"
		"Connection: keep-alive\r\n"
		"Keep-Alive: timeout=60\r\n"
		"Server: FastServer\r\n"
		"Data: Grace_Kelly.ogg\r\n"
		"Content-Type: text/csv\r\n"
		"\r\n";

	string str(request_ok1+ std::to_string(content.length()) + request_ok2);

	//str.append("data.jpg;Grace_Kelly.ogg;0.64\r\ndata1.jpg;Grace_Kelly1.ogg;0.65\r\n");
	str.append(content);

	return str;
}



HttpRequest::HttpRequest(char* resend_http, size_t len)
{
	if (len == 0) return;

	//cout << resend_http << endl;

	//http_request = string(resend_http);	// Признак конца файла (?)
	for (int i = 0; i < len; i++)
		http_request += resend_http[i];

	//cout << http_request << endl;

	http_size = len;
}


// Алгоритм парсинга http
// 1 - Парсим метод запрос
// 1.1 - Определяем корректность запроса
// 1.2 - Выделяем метод запроса
// 2 - Выделяем заголовки запроса и парсим
// 3 - Выделяем тело запроса
// 3.1 - Из тела запроса выделяем заголовки
// 3.2 - И выделяем данные

bool HttpRequest::parserMethod()
{
	string::size_type n;
	//string::size_type m;
	//string::size_type c;

	// Выделение первой строки c методом
	n = http_request.find("\n");
	string line0 = http_request.substr(0, n);

	if (line0.find("POST") != string::npos) {
		method = "POST";
		return true;
	}
	else if (line0.find("GET") != string::npos) {
		method = "GET";	// Метод GET не поддерживается!
		return false;
	}
	else if (line0.find("OPTIONS") != string::npos) {
		method = "OPTIONS";
		return true;
	}
	else
		return false;
}


// ACHUNG! Быдлокод
bool HttpRequest::parser()
{
	string::size_type n;
	string::size_type m;
	string::size_type c;

	// Выделение заголовков
	n = http_request.find("\r\n") + 2;
	m = http_request.find("\r\n\r\n");	// До разделителя заголовка от тела
	http_headers = http_request.substr(n, m - n);

	http_body = http_request.substr(m + 4);	// Выделение тела запроса исключая перенос \r\n\r\n

	m = 0;
	n = http_headers.find("\r\n");
	while (n != string::npos) {
		string line = http_headers.substr(m, n - m);

		replace_all(line, " ", "");

		c = line.find(":");
		if (c != string::npos) {
			string header = line.substr(0, c);
			string value = line.substr(c + 1);
			//cout << "[HTTP header:value]:" << header << ":" << value << endl;

			// Получем размер тела (и проверяем на корректность)
			if (header.find("Content-Length") != string::npos) {
				content_length = value;
				try {
					content_size = stoi(content_length);
				}
				catch (exception e) {
					return false;
				}
			}

			// Определения данных
			if (header.find("Content-Type") != string::npos) {
				content_type = value;

				// Поиск разделителей body
				string::size_type s = content_type.find(";");
				if (s != string::npos) {
					meta = content_type.substr(0, s);
					boundary = content_type.substr(s + 1);

					// Выделения кода bondary
					string::size_type b = boundary.find("=");
					string::size_type b1 = boundary.find("boundary=");
					if (b != string::npos && b1 != string::npos)
						boundary = boundary.substr(b + 1);
				}

			}

		}

		m = n + 2;							// Сдвиг начала
		n = http_headers.find("\r\n", m);	// Сдвиг конца
	}


	// Выделение подзаголовков изображения
	m = http_body.find(boundary);

	n = http_body.find("\r\n", m) + 2;	// Следующая строка

	m = http_body.find("\r\n\r\n", n);	// Далее Данные изображения

										// Между n и m заключены заголовки изображения
	http_header_image = http_body.substr(n, m - n);
	parserHeadersBody();
	//	cout << http_header_image << endl; // Верно



	//Выделение в body бинарных данных
	n = m + 4;	// Далее Данные изображения
	c = http_body.find(boundary, n);

	string buffer = http_body.substr(n, c - n);
	body_len = buffer.length()-4;	///// Хрень хз почему так
	body = new Uint8[body_len];

	for (int i = 0; i < body_len; i++) {
		body[i] = buffer[i];
	}

	return true;
}


// Парсит дополнительные заголовки данных(изображения), и получает тип даннх
bool HttpRequest::parserHeadersBody() {
	if (http_header_image.length() == 0)
		return false;

	string::size_type n;
	string::size_type m;
	string::size_type c;

	m = 0;
	n = http_header_image.find("\r\n");
	while (n != string::npos) {
		string line = http_header_image.substr(m, n - m);

		// Удаление всех пробелов!!
		replace_all(line, " ", "");

		c = line.find(":");
		// Выделение значения
		if (c != string::npos) {
			string header = line.substr(0, c);
			string value = line.substr(c + 1);

			// Данные об изображении и находи имя
			if (header.find("Content-Disposition") != string::npos) {
				string::size_type nm_start;
				string::size_type nm_end;
				// Пытаемся выделить название изображения
				nm_start = value.find("filename=\"") + 10;
				nm_end = value.find("\"", nm_start);
				// Между nm_start и nm_end название файла
				http_body_name = value.substr(nm_start, nm_end - nm_start);
				// (12.54:11.11.17) Правка выделяем расширение файла
				nm_start = http_body_name.rfind(".") + 1;
				http_body_content_type = http_body_name.substr(nm_start);

				// Проверка на корректность
				if (http_body_content_type.length() != 0) {
					if (http_body_content_type == "jpg" || http_body_content_type == "png") {
						type = 0;
					}
					else if (http_body_content_type == "wav" || http_body_content_type == "ogg" || http_body_content_type == "flac") {
						type = 1;
					}
				}

			}
		}

		m = n + 2;								// Сдвиг начала + перенос строки
		n = http_header_image.find("\r\n", m);	// Сдвиг конца
	}

	return true;
}

// Парсим заголовки http запроса (10.11.17)
bool HttpRequest::parserHeaders() {
	string::size_type m;
	string::size_type n;
	string::size_type c;
	string line;
	m = 0;
	while ((n = http_headers.find("\r\n")) != string::npos) {
		line = http_headers.substr(m, n - m);	// Выделяем линию заголовка

												// Удаление всех пробелов
		replace_all(line, " ", "");

		// По разделителю находим Header и его Value
		c = line.find(":");
		if (c != string::npos) {
			string header_name = line.substr(0, c);
			string header_value = line.substr(c + 1);							// Удаление разделителя ':'
			headers.insert(pair<string, string>(header_name, header_value));	// Заносим значения в словарь
		}
		else {
			cout << "[ERROR]: uncorrect header" << endl;
			return false;
		}

	}

	return true;
}

void  HttpRequest::saveBody(string fileName) {

	if (body_len > 0) {
		ofstream out(fileName, ios::binary);  //Ставим режим "бинарный файл"
		for (int i = 0; i < body_len; i++) {
			out.write((char *)&body[i], sizeof(Uint8));
		}
		out.close();
	}
	else {
		cout << "[body_len]: " << body_len << endl;
	}


}

// Из памяти создает изображение sf::Image
void  HttpRequest::saveImagesFromMemory(string fileName) {
	Image img;
	// bmp, png, tga, jpg, gif, psd, hdr
	if (img.loadFromMemory(body, body_len)) {

		SoundBuffer audio_from_img = ImageToMusicY(img);
		audio_from_img.saveToFile(fileName + ".ogg");
		img.saveToFile(fileName);
		
		// Получаем сжатие изображения
		Image mini_img = ResizeImg(img);

		// Получаем средний цвет
		getPixelsDiff(mini_img);

	}
	else
		cout << "[saveImagesFromMemory]: error create image from file" << endl;

	// Для дальнейшей обработки может понадобится:
	// https://www.sfml-dev.org/documentation/2.4.2/classsf_1_1Image.php#ac6137a608a9efaae2735c13ff259c214
}


void mkdir(Color color, Image imgsave, string namefile);

// Из памяти создает аудио буффер sf::Image
void  HttpRequest::saveMusicFromMemory(string fileName) {
	SoundBuffer buffer;
	// Подгрузка аудио из памяти
	if (buffer.loadFromMemory(body, body_len)) {

		const Int16 *source = buffer.getSamples();  // Отправляем ссылку на область памяти звука
		size_t len = buffer.getSampleCount();		// Колличество симплов

		// Перевод по алгоритму влада изображения в аудио
		Image img_from_audio = MusicToImageY(buffer);
		// Сохраняем полуенное изображение
		//img_from_audio.saveToFile("audio//"+fileName + ".png");
		// Сохраняем полученное изображение
		//buffer.saveToFile(fileName);

		// Получаем сжатие изображения
		Image mini_img = ResizeImg(img_from_audio);

		// Получаем средний цвет
		Color med_color = getPixelsDiff(mini_img);
		// Создание каталога по цвету
		mkdir(med_color, mini_img, fileName);



	}
	else {
		cout << "[saveMusicFromMemory]: error create audio from file" << endl;
	}

}

void  HttpRequest::showHeaders() {
	cout << "[HTTP method]:" << method << endl;
	//cout << "[HTTP headers]:" << http_headers << endl;
	cout << "[HTTP content_length]:" << content_size << endl;
	cout << "[HTTP content_type]:" << content_type << endl;
	cout << "[HTTP meta]:" << meta << endl;
	cout << "[HTTP boundary]:" << boundary << endl;
	cout << "[HTTP file name]:" << http_body_name << endl;
	cout << "[HTTP file type]:" << http_body_content_type << endl;
}

// Утечка памяти
HttpRequest::~HttpRequest()
{
	delete[] body;
}