#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace sf;

Color func_ColorFromMusic(sf::Uint16 x0) {
	//x0 -= 32767;
	Uint16 x = x0 / 2;

	Color color;

	color.r = (x / 1024) * 255 / 31;
	color.g = (x % 1024) / 32 * 255 / 31;
	color.b = (x % 32) * 255 / 31;
	color.a = 255;



	return color;
}

Uint16 func_MusicFromColor(Color color) {
	float f_red = floor((color.r * 31 / 255.0f) + 0.5f) * 1024;
	float f_green = floor((color.g * 31 / 255.0f) + 0.5f) * 32;
	float f_blue = floor((color.b * 31 / 255.0f) + 0.5f);

	Uint16 x = 2 * (Uint16)(f_red + f_green + f_blue);

	//cout << "[x]: " << x << endl;
	return x;
}

Image MusicToImage(SoundBuffer &buffer) {

	const Int16 *source = buffer.getSamples();  // Отправляем ссылку на область памяти звука
	size_t len = buffer.getSampleCount();		// Колличество симплов
												// Поиск максимума
	Uint16 m = ((int)source[0]) + 32767;
	Uint16 m1 = ((int)source[0]) + 32767;
	int im = 0;
	int im1 = 0;
	for (int i = 1; i < len; i++) {
		if (m < ((int)source[i]) + 32767) {
			m = ((int)source[i]) + 32767;
			im = i;
		}
		if (m1 > ((int)source[i]) + 32767) {
			m1 = ((int)source[i]) + 32767;
			im1 = i;
		}
	}
	std::cout << "[MAX AUDIO SIMPLE!!!]:" << m << "\t[i]:" << im << std::endl;
	std::cout << "[MIN AUDIO SIMPLE!!!]:" << m1 <<"\t[i]:" << im << std::endl;
	std::cout << "[LEN SIMPLE]:" << len << std::endl;
	std::cout << "[SampleRate]:" << buffer.getSampleRate() << std::endl;
	std::cout << "[ChannelCount]:" << buffer.getChannelCount() << std::endl;

	// Сторона изображения
	int n = (int)sqrt(len);	// Уточнить у влада про округление

	Image image;
	image.create(n, n, Color(0, 0, 0));

	// инменение(?)
	int it = 0;

	Int16 *tmp = new Int16[n];
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			image.setPixel(i, j, func_ColorFromMusic(((int)source[it]) + 32767));	//14:05 11.11.17
			it++;
		}
	}

	return image;
}

SoundBuffer ImageToMusic(Image &image) {

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
			dst.setPixel(i, j, image.getPixel(i, j));
			data[it] = ((int)func_MusicFromColor(image.getPixel(i, j)) - 32767);
			it++;
		}
	}

	SoundBuffer sb;
	std::cout << "[len buffer]:" << n << std::endl;
	sb.loadFromSamples(data, n, 2, 44100);

	delete[] data;



	return sb;

}

Image ResizeImg(Image src) {
	
	Image dst;

	int m = 300;
	dst.create(m, m);

	int d =  src.getSize().x / m;

	int count = d*d;
	

	for (int i1 = 0; i1 < m; i1++) {
		for (int j1 = 0; j1 < m; j1++) {
			int sumR = 0;
			int sumG = 0;
			int sumB = 0;

			for (int i = 0; i < d; i++) {
				for (int j = 0; j < d; j++) {
					sumR += src.getPixel(i + i1*d, j + j1*d).r;
					sumG += src.getPixel(i + i1*d, j + j1*d).g;
					sumB += src.getPixel(i + i1*d, j + j1*d).b;
				}
			}
			sumR /= count;
			sumG /= count;
			sumB /= count;

			dst.setPixel(i1, j1, Color(sumR, sumG, sumB));

		}
	}

	//dst.saveToFile("IMAGESSSSS.png");

	return dst;
}



// Средний цвет => Средний цвет
Color getPixelsDiff(Image src) {

	// 1 Пункт
	const Uint8* ptr = src.getPixelsPtr();
	int len = src.getSize().x * src.getSize().y * 4;
	int n = len/4;	// Число викселей


	// Djpvj;yf nhf,kv enjxybnm
	Uint32 sumR = 0;
	Uint32 sumG = 0;
	Uint32 sumB = 0;

	for (int i = 0; i < len; i+=4) {
		sumR += ptr[i];
		sumG += ptr[i+1];
		sumB += ptr[i+2];
	}

	sumR /= n;
	sumG /= n;
	sumB /= n;

	std::cout << "[mediun color]: " << sumR << " " << sumG << " " << sumB << std::endl;

	return Color(sumR, sumG, sumB);
}



Color getPixelsDiffHSV(Image src) {

	// 1 Пункт
	
	Uint32 sumR = 0;
	Uint32 sumG = 0;
	Uint32 sumB = 0;

	int n = src.getSize().x * src.getSize().y;

	/*for (int i = 0; i < n; i ++) {
		src.
	}*/

	sumR /= n;
	sumG /= n;
	sumB /= n;

	std::cout << "[mediun color]: " << sumR << " " << sumG << " " << sumB << std::endl;

	return Color(sumR, sumG, sumB);
}