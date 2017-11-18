#pragma once
#include <cmath>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

class ColorHSV {
public:
	Uint16 h;
	Uint8 s;
	Uint8 v;

	ColorHSV(Uint16 _h, Uint8 _s, Uint8 _v) {
		h = _h;
		s = _s;
		v = _v;
	}
};

ColorHSV rgb2hsv(Color color)
{
	// Ищем максимальное значение
	Uint8 MAX = max(color.r, max(color.g, color.b));
	Uint8 MIN = min(color.r, max(color.g, color.b));

	Uint16 H;
	Uint8 S;
	Uint8 V;

	if (MAX == MIN)
		H = 0;
	else if (MAX == color.r && color.g >= color.b)
		H = 60 * (color.g - color.b) / (MAX - MIN) + 0;
	else if (MAX == color.r && color.g < color.b)
		H = 60 * (color.g - color.b) / (MAX - MIN) + 360;
	else if (MAX == color.g)
		H = 60 * (color.b - color.r) / (MAX - MIN) + 120;
	else if (MAX == color.b)
		H = 60 * (color.b - color.g) / (MAX - MIN) + 240;

	if (MAX == 0)
		S = 0;
	else
		S = 1 - MIN / MAX;

	V = MAX;

	return ColorHSV(H, S, V);
}


Color hsv2rgb(ColorHSV colorhsv)
{
	Uint8 Hi = (colorhsv.h / 60) % 6;

	Uint8 Vmin = (float)((100 - colorhsv.s)* colorhsv.v) / 100.0f;

	Uint8 a = (float)(colorhsv.v - Vmin)*(float)((colorhsv.h % 60) / 60.0f);

	Uint8 Vinc = Vmin + a;
	Uint8 Vdes = colorhsv.v - a;

	Color color;
	if (Hi == 0) {
		color.r = colorhsv.v;
		color.g = Vinc;
		color.b = Vmin;
	}
	else if (Hi == 1) {
		color.r = Vdes;
		color.g = colorhsv.v;
		color.b = Vmin;
	}
	else if (Hi == 2) {
		color.r = Vmin;
		color.g = colorhsv.v;
		color.b = Vinc;
	}
	else if (Hi == 3) {
		color.r = Vmin;
		color.g = Vdes;
		color.b = colorhsv.v;
	}
	else if (Hi == 4) {
		color.r = Vinc;
		color.g = Vmin;
		color.b = colorhsv.v;
	}
	else if (Hi == 5) {
		color.r = colorhsv.v;
		color.g = Vmin;
		color.b = Vdes;
	}
	// Из процентов в значения
	color.r = ceil(color.r * 255 / 100.0f);
	color.g = ceil(color.g * 255 / 100.0f);
	color.b = ceil(color.b * 255 / 100.0f);

	return color;
}



Uint16 funcY_MusicFromColor(Color color) {

	ColorHSV colorhsv = rgb2hsv(color);

	//tmp
	Uint8 x = colorhsv.h * 255 / 360;
	Uint8 y = colorhsv.s - 85;
	Uint8 z = colorhsv.v - 85;

	Uint16 S = (16*z+y)*256+x;

	return S;
}

Color funcY_MusicToColor(Uint16 s0) {

	//s0 = s0 >> 8;

	Uint8 x = s0 % 256;
	Uint8 y = ((s0 >> 8) % 16);	// ??
	Uint8 z = s0 >> 12;

	// Временно для отладки
	Uint16 H = x * 360 / 255;
	Uint8 S = 85 + y;
	Uint8 V = 85 + z;
	

	return  hsv2rgb(ColorHSV(H, S, V));
}


Image MusicToImageY(SoundBuffer &buffer) {

	const Int16 *source = buffer.getSamples();  // Отправляем ссылку на область памяти звука
	size_t len = buffer.getSampleCount();		// Колличество симплов
	
	Int16* fastData = new Int16[len];
	for (int i = 0; i < len; i++) {
		fastData[i] = source[i];
	}


	// Поиск максимума
	Int16 m = (fastData[0]);
	Int16 m1 = (fastData[0]) ;
	int im = 0;
	int im1 = 0;
	for (int i = 1; i < len; i++) {
		if (m < fastData[i])  {
			m = fastData[i] ;
			im = i;
		}
		if (m1 > fastData[i]) {
			m1 = fastData[i];
			im1 = i;
		}
	}
	std::cout << "[MAX AUDIO SIMPLE!!!]:" << m << "\t[i]:" << im << std::endl;
	std::cout << "[MIN AUDIO SIMPLE!!!]:" << m1 << "\t[i]:" << im1 << std::endl;
	std::cout << "[LEN SIMPLE]:" << len << std::endl;
	std::cout << "[SampleRate]:" << buffer.getSampleRate() << std::endl;
	std::cout << "[ChannelCount]:" << buffer.getChannelCount() << std::endl;

	// Сторона изображения
	int n = (int)sqrt(len);

	Image image;
	image.create(n, n, Color(0, 0, 0));

	// 
	int it = 0;
	Int16 *tmp = new Int16[n];
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			image.setPixel(i, j, funcY_MusicToColor(source[it] + 32768) );	//14:05 11.11.17
			it++;
		}
	}

	return image;
}

SoundBuffer ImageToMusicY(Image &image) {

	// Выравниваем изображение по квадрату
	int width = image.getSize().x;
	int height = image.getSize().y;

	Image dst;
	int n = 0;	// Размер предпологаемого массива
	int side = 0;	// Размер предпологаемого массива
	int it = 0;
	Int16* data;

	// Отсекаем часть изображения по длинне
	if (width > height) {
		dst.create(height, height, Color(0, 0, 0));
		n = height*height;
		side = height;
	}
	else if (width <= height) {
		dst.create(width, width, Color(0, 0, 0));
		n = width*width;
		side = width;
	}

	data = new Int16[n];
	for (int i = 0; i < side; i++) {
		for (int j = 0; j < side; j++) {
			//dst.setPixel(i, j, image.getPixel(i, j));
			data[it] = funcY_MusicFromColor(image.getPixel(i, j) );
			it++;
		}
	}

	SoundBuffer sb;
	std::cout << "[len buffer]:" << n << std::endl;
	sb.loadFromSamples(data, n, 2, 44100);

	delete[] data;
	return sb;

}

