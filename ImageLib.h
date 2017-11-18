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

// "Тупой" алгоритм
double ImagesAndImages(Image img1, Image img2) {

	int n = img1.getSize().x*img2.getSize().y;	// 90000 = 300*300

	int m = img1.getSize().x;	// Сторона

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

	// ОТКЛОНЕНИЕ
	float Ei = f / (255.0f * sqrt(3));

	return Ei;
}

void mkdir(Color color, Image imgsave, string namefile) {

	int shift = 2;

	string dir = "audio/" + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b);


	if (CreateDirectory(dir.c_str(), NULL)) {
		// Если не создана дирректория
		cout << "[create dir]: " << dir << endl;
		imgsave.saveToFile(dir+"//"+namefile + ".png");

		// Вставить функцию что похожего нет ((
		// ...

	}
	else {
		// Если создана
		imgsave.saveToFile(dir + "//" + namefile + ".png");

		// Вставить функцию поиска
		deviationImg(dir, imgsave, namefile);
	}
	
}

// Отклонение (где ищем, пришедшее изображение)
void deviationImg(string dir , Image imgsave, string name_file) {

	csv = "";


	WIN32_FIND_DATA FindFilesData;

	HANDLE hf;

	string fulldir(dir + "\\*.png");

	// Поиск изображений в данной дирректории
	hf = FindFirstFile(fulldir.c_str(), &FindFilesData);

	double E = 0.0;
	int count_check_muz = 0;

	

	if (hf != INVALID_HANDLE_VALUE) {
		do {
			std::cout << FindFilesData.cFileName << endl;


			Image img_from_bd;
			// Подгрузка изображений с hard
			if (img_from_bd.loadFromFile(dir +"/"+ FindFilesData.cFileName)) {

				// Вызов функции сравнеия по средним квадратам (++) 15:12 12.11.17
				/*data_coef.insert(std::to_string(ImagesAndImages(imgsave, img_from_bd)));

				// Подготовка данных
				data_audio.insert(FindFilesData.cFileName);*/

				string line(dir + "/" + FindFilesData.cFileName);
				string line2(FindFilesData.cFileName);

				// ОТКЛОНЕНИЕ
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

// Святая дева котия и иисусец котец, как эти функции еще называть?
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

			// Подгрузка аудио из памяти
			if (buffer.loadFromFile(path)) {

				const Int16 *source = buffer.getSamples();  // Отправляем ссылку на область памяти звука
				size_t len = buffer.getSampleCount();		// Колличество симплов

				// Перевод по алгоритму влада изображения в аудио
				Image img_from_audio = MusicToImage(buffer);
				// Сохраняем полуенное изображение
				//img_from_audio.saveToFile(fileName + ".png");
				// Сохраняем полученное изображение
				//buffer.saveToFile(fileName);

				// Получаем сжатие изображения
				Image mini_img = ResizeImg(mini_img);

				// Получаем средний цвет
				Color med_color = getPixelsDiff(img_from_audio);
				// Создание каталога по цвету
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