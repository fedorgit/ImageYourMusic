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
	string http_request;	// ������ �� �������
	string http_headers;	// ��������� �������
	string http_body;		// ���� �������
	size_t http_size;		// ������ http ������� � ������ 
public:
	string method;			// ������: {GET, POST, OPTIONS}
private:
	string content_length;	// ������ ���� �������
	size_t content_size = -1;	// ������ ���� �������
	string content_type;	// Content-Type [example]:Content-Type:multipart/form-data; boundary=----WebKitFormBoundaryGURBsmZPmneAAiQ3
	string meta;			// ��� ������
	string boundary;		// ����������� �������� ������

	// 10.11.17
	map <string, string> headers;

	string http_header_image;	// ��������� �����������. Example:
								// Content-Disposition: form-data; name="AttachedFile1"; filename="horror-photo-1.jpg"
								// Content-Type: image/jpeg
public:
	string http_body_name;			// ���������� �������� �����
	string http_body_content_type;	// ���������� ����� (���� ��� ���������� �� �������) {����������(jpg, png), �����(ogg, wav, flac)} 
	int type = -1;					// ��� ������ (���� ��� ���������� �� �������) {����������(jpg, png), �����(ogg, wav, flac)} 
private:
	Uint8* body;				// ��� ����� ������ ���� ���-�� �� ����� �������� �� ������ ���
	size_t body_len = -1;		// ��

public:
	HttpRequest(char* resend_http, size_t len);
	bool parserMethod();
	bool parser();
	bool parserHeaders();
	bool parserHeadersBody();	// �������� ������������ body {Image, Music}
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

	//http_request = string(resend_http);	// ������� ����� ����� (?)
	for (int i = 0; i < len; i++)
		http_request += resend_http[i];

	//cout << http_request << endl;

	http_size = len;
}


// �������� �������� http
// 1 - ������ ����� ������
// 1.1 - ���������� ������������ �������
// 1.2 - �������� ����� �������
// 2 - �������� ��������� ������� � ������
// 3 - �������� ���� �������
// 3.1 - �� ���� ������� �������� ���������
// 3.2 - � �������� ������

bool HttpRequest::parserMethod()
{
	string::size_type n;
	//string::size_type m;
	//string::size_type c;

	// ��������� ������ ������ c �������
	n = http_request.find("\n");
	string line0 = http_request.substr(0, n);

	if (line0.find("POST") != string::npos) {
		method = "POST";
		return true;
	}
	else if (line0.find("GET") != string::npos) {
		method = "GET";	// ����� GET �� ��������������!
		return false;
	}
	else if (line0.find("OPTIONS") != string::npos) {
		method = "OPTIONS";
		return true;
	}
	else
		return false;
}


// ACHUNG! ��������
bool HttpRequest::parser()
{
	string::size_type n;
	string::size_type m;
	string::size_type c;

	// ��������� ����������
	n = http_request.find("\r\n") + 2;
	m = http_request.find("\r\n\r\n");	// �� ����������� ��������� �� ����
	http_headers = http_request.substr(n, m - n);

	http_body = http_request.substr(m + 4);	// ��������� ���� ������� �������� ������� \r\n\r\n

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

			// ������� ������ ���� (� ��������� �� ������������)
			if (header.find("Content-Length") != string::npos) {
				content_length = value;
				try {
					content_size = stoi(content_length);
				}
				catch (exception e) {
					return false;
				}
			}

			// ����������� ������
			if (header.find("Content-Type") != string::npos) {
				content_type = value;

				// ����� ������������ body
				string::size_type s = content_type.find(";");
				if (s != string::npos) {
					meta = content_type.substr(0, s);
					boundary = content_type.substr(s + 1);

					// ��������� ���� bondary
					string::size_type b = boundary.find("=");
					string::size_type b1 = boundary.find("boundary=");
					if (b != string::npos && b1 != string::npos)
						boundary = boundary.substr(b + 1);
				}

			}

		}

		m = n + 2;							// ����� ������
		n = http_headers.find("\r\n", m);	// ����� �����
	}


	// ��������� ������������� �����������
	m = http_body.find(boundary);

	n = http_body.find("\r\n", m) + 2;	// ��������� ������

	m = http_body.find("\r\n\r\n", n);	// ����� ������ �����������

										// ����� n � m ��������� ��������� �����������
	http_header_image = http_body.substr(n, m - n);
	parserHeadersBody();
	//	cout << http_header_image << endl; // �����



	//��������� � body �������� ������
	n = m + 4;	// ����� ������ �����������
	c = http_body.find(boundary, n);

	string buffer = http_body.substr(n, c - n);
	body_len = buffer.length()-4;	///// ����� �� ������ ���
	body = new Uint8[body_len];

	for (int i = 0; i < body_len; i++) {
		body[i] = buffer[i];
	}

	return true;
}


// ������ �������������� ��������� ������(�����������), � �������� ��� �����
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

		// �������� ���� ��������!!
		replace_all(line, " ", "");

		c = line.find(":");
		// ��������� ��������
		if (c != string::npos) {
			string header = line.substr(0, c);
			string value = line.substr(c + 1);

			// ������ �� ����������� � ������ ���
			if (header.find("Content-Disposition") != string::npos) {
				string::size_type nm_start;
				string::size_type nm_end;
				// �������� �������� �������� �����������
				nm_start = value.find("filename=\"") + 10;
				nm_end = value.find("\"", nm_start);
				// ����� nm_start � nm_end �������� �����
				http_body_name = value.substr(nm_start, nm_end - nm_start);
				// (12.54:11.11.17) ������ �������� ���������� �����
				nm_start = http_body_name.rfind(".") + 1;
				http_body_content_type = http_body_name.substr(nm_start);

				// �������� �� ������������
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

		m = n + 2;								// ����� ������ + ������� ������
		n = http_header_image.find("\r\n", m);	// ����� �����
	}

	return true;
}

// ������ ��������� http ������� (10.11.17)
bool HttpRequest::parserHeaders() {
	string::size_type m;
	string::size_type n;
	string::size_type c;
	string line;
	m = 0;
	while ((n = http_headers.find("\r\n")) != string::npos) {
		line = http_headers.substr(m, n - m);	// �������� ����� ���������

												// �������� ���� ��������
		replace_all(line, " ", "");

		// �� ����������� ������� Header � ��� Value
		c = line.find(":");
		if (c != string::npos) {
			string header_name = line.substr(0, c);
			string header_value = line.substr(c + 1);							// �������� ����������� ':'
			headers.insert(pair<string, string>(header_name, header_value));	// ������� �������� � �������
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
		ofstream out(fileName, ios::binary);  //������ ����� "�������� ����"
		for (int i = 0; i < body_len; i++) {
			out.write((char *)&body[i], sizeof(Uint8));
		}
		out.close();
	}
	else {
		cout << "[body_len]: " << body_len << endl;
	}


}

// �� ������ ������� ����������� sf::Image
void  HttpRequest::saveImagesFromMemory(string fileName) {
	Image img;
	// bmp, png, tga, jpg, gif, psd, hdr
	if (img.loadFromMemory(body, body_len)) {

		SoundBuffer audio_from_img = ImageToMusicY(img);
		audio_from_img.saveToFile(fileName + ".ogg");
		img.saveToFile(fileName);
		
		// �������� ������ �����������
		Image mini_img = ResizeImg(img);

		// �������� ������� ����
		getPixelsDiff(mini_img);

	}
	else
		cout << "[saveImagesFromMemory]: error create image from file" << endl;

	// ��� ���������� ��������� ����� �����������:
	// https://www.sfml-dev.org/documentation/2.4.2/classsf_1_1Image.php#ac6137a608a9efaae2735c13ff259c214
}


void mkdir(Color color, Image imgsave, string namefile);

// �� ������ ������� ����� ������ sf::Image
void  HttpRequest::saveMusicFromMemory(string fileName) {
	SoundBuffer buffer;
	// ��������� ����� �� ������
	if (buffer.loadFromMemory(body, body_len)) {

		const Int16 *source = buffer.getSamples();  // ���������� ������ �� ������� ������ �����
		size_t len = buffer.getSampleCount();		// ����������� �������

		// ������� �� ��������� ����� ����������� � �����
		Image img_from_audio = MusicToImageY(buffer);
		// ��������� ��������� �����������
		//img_from_audio.saveToFile("audio//"+fileName + ".png");
		// ��������� ���������� �����������
		//buffer.saveToFile(fileName);

		// �������� ������ �����������
		Image mini_img = ResizeImg(img_from_audio);

		// �������� ������� ����
		Color med_color = getPixelsDiff(mini_img);
		// �������� �������� �� �����
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

// ������ ������
HttpRequest::~HttpRequest()
{
	delete[] body;
}