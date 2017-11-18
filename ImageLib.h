#pragma once
#include <cmath>
#include <iostream>


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>

using namespace std;
using namespace sf;


Image ResizeImg(Image src);
Image MusicToImage(SoundBuffer &buffer);
Color getPixelsDiff(Image src);
void deviationImg(string dir, Image imgsave, string name_file);

// "�����" ��������
double ImagesAndImages(Image img1, Image img2) {

	int n = img1.getSize().x*img2.getSize().y;	// 90000 = 300*300

	int m = img1.getSize().x;	// �������

	//double *f = new double[n];

	double f = 0;

	int it = 0;
	for(int i = 0; i < m; i++)
		for (int j = 0; j < m; j++) {
			f += sqrt((int)pow(img1.getPixel(i, j).r - (int)img2.getPixel(i, j).r, 2) +
				pow((int)img1.getPixel(i, j).g - (int)img2.getPixel(i, j).g, 2) +
				pow((int)img1.getPixel(i, j).b - (int)img2.getPixel(i, j).b, 2));
			it++;
		}
			
	f /= n;

	// ����������
	float Ei = f / (255.0f * sqrt(3));

	return Ei;
}

void mkdir(Color color, Image imgsave, string namefile) {

	int shift = 2;

	string dir = "audio/" + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b);


	if (CreateDirectory(dir.c_str(), NULL)) {
		// ���� �� ������� �����������
		cout << "[create dir]: " << dir << endl;
		imgsave.saveToFile(dir+"//"+namefile + ".png");

		// �������� ������� ��� �������� ��� ((
		// ...

	}
	else {
		// ���� �������
		imgsave.saveToFile(dir + "//" + namefile + ".png");

		// �������� ������� ������
		deviationImg(dir, imgsave, namefile);
	}
	
}

// ���������� (��� ����, ��������� �����������)
void deviationImg(string dir , Image imgsave, string name_file) {

	csv = "";


	WIN32_FIND_DATA FindFilesData;

	HANDLE hf;

	string fulldir(dir + "\\*.png");

	// ����� ����������� � ������ �����������
	hf = FindFirstFile(fulldir.c_str(), &FindFilesData);

	double E = 0.0;
	int count_check_muz = 0;

	

	if (hf != INVALID_HANDLE_VALUE) {
		do {
			std::cout << FindFilesData.cFileName << endl;


			Image img_from_bd;
			// ��������� ����������� � hard
			if (img_from_bd.loadFromFile(dir +"/"+ FindFilesData.cFileName)) {

				// ����� ������� �������� �� ������� ��������� (++) 15:12 12.11.17
				/*data_coef.insert(std::to_string(ImagesAndImages(imgsave, img_from_bd)));

				// ���������� ������
				data_audio.insert(FindFilesData.cFileName);*/

				string line(dir + "/" + FindFilesData.cFileName);
				string line2(FindFilesData.cFileName);

				// ����������
				double E = ImagesAndImages(imgsave, img_from_bd);

				csv.append(line +";" + line2 + ";"+ std::to_string(1.0 - E*2*3.14) +"\r\n" );



			}

		} while (FindNextFile(hf, &FindFilesData) != 0);
		FindClose(hf);
	}
	else {
		std::cout << "[Error see files]" << endl;
	}


}


// (?)
BOOL FileExists(string szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// ������ ���� ����� � ������� �����, ��� ��� ������� ��� ��������?
string get_mkdir_color(Color color) {

	string dir = "audio/" + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b);

	if (FileExists(dir)) {
		return dir;
	}
	else {
		return "";
	}
}






void db_convert() {
	WIN32_FIND_DATA FindFilesData;

	HANDLE hf;
	hf = FindFirstFile("dbm\\*.ogg", &FindFilesData);

	if (hf != INVALID_HANDLE_VALUE) {
		do {
			std::cout << FindFilesData.cFileName << endl;

			SoundBuffer buffer;

			string path("dbm\\");
			path.append(FindFilesData.cFileName);

			// ��������� ����� �� ������
			if (buffer.loadFromFile(path)) {

				const Int16 *source = buffer.getSamples();  // ���������� ������ �� ������� ������ �����
				size_t len = buffer.getSampleCount();		// ����������� �������

				// ������� �� ��������� ����� ����������� � �����
				Image img_from_audio = MusicToImage(buffer);
				// ��������� ��������� �����������
				//img_from_audio.saveToFile(fileName + ".png");
				// ��������� ���������� �����������
				//buffer.saveToFile(fileName);

				// �������� ������ �����������
				Image mini_img = ResizeImg(mini_img);

				// �������� ������� ����
				Color med_color = getPixelsDiff(img_from_audio);
				// �������� �������� �� �����
				mkdir(med_color, mini_img, FindFilesData.cFileName);

			}
			else {
				cout << "[saveMusicFromMemory]: error create audio from file" << endl;
			}

		} while (FindNextFile(hf, &FindFilesData) != 0);
		FindClose(hf);
	}
	else {
		std::cout << "[Error see files]" << endl;
	}
}