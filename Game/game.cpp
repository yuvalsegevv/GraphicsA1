#include "game.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define set_val(ARR, INDEX, val) ARR[INDEX] = val; ARR[INDEX+1] = val; ARR[INDEX+2] = val; ARR[INDEX+3] = 255;

static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init()
{		

	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");
	
	
	AddTexture("../res/textures/lena256.jpg", false);
	//AddTexture("../res/textures/box0.bmp", false);
	AddShape(Plane,-1,TRIANGLES);
	Task4();
	Task5();
	Task6();
	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,10);
	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void calc(unsigned char* data, unsigned char amount, int i, int j, int w, int h) {
	int x = data[4 * i * w + 4 * j] + amount;
	set_val(data, 4 * i * w + 4 * j, x);
}
//
void WriteToTxt(unsigned char* output, int w, int h, int div, std::string name) {
	std::ofstream txt_file;
	txt_file.open(name, std::ofstream::out);
	if (txt_file.is_open()) {
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				txt_file << (int)output[4 * i * w + 4 * j] / div << ",";
			}
			txt_file << '\n';
		}
	}
	else {
		std::cout << "error in opening file" << std::endl;
	}
	txt_file.close();
}
void Game::Task6() {
	int w, h, components;
	double a, d, b, g;
	unsigned char* data = stbi_load("../res/textures/lena256.jpg", &w, &h, &components, 4);
	g = (double)5 / 16;
	a = (double)7 / 16;
	b = (double)3 / 16;
	d = (double)1 / 16;
	auto* return_val = (unsigned char*)malloc(h * w * 4 * sizeof(unsigned char));
	memcpy(return_val, data, 4 * w * h * sizeof(unsigned char));
	for (int i = 0; i < h - 1; i++) {
		for (int j = 0; j < w - 1; j++) {
			int index = 4 * i * w + 4 * j;
			int p = (data[index] / 16) * 16;
			int e = data[index] - p;
			set_val(return_val, index, p)

				calc(data, a * e, i, j + 1, w, h);
			calc(data, b * e, i + 1, j - 1, w, h);
			calc(data, g * e, i + 1, j, w, h);
			calc(data, d * e, i + 1, j + 1, w, h);
		}
	}
	WriteToTxt(return_val, w, h, 16,"../assignment/img6.txt");
	AddTexture(w, h, return_val);
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if(isActive)
	{
	}
}

Game::~Game(void)
{
}



void setPixel(unsigned char* data, int i, int j, int value)
{
	for (int k = 0; k < 3; k++)
		data[i * 4 + j * 256 * 4 + k] = value;
	data[i * 4 + j * 256 * 4 + 3] = 255;
}

unsigned char getPixel(unsigned char* data, int i, int j)
{
	return data[i * 4 + j * 256 * 4];
}

void setPixel(int* data, int i, int j, int value)
{
	data[i + j * 256] = value;
}

int getPixel(int* data, int i, int j)
{
	return data[i + j * 256];
}

int* picture_to_int_array(unsigned char* data)
{
	int* n_data = (int*)malloc(sizeof(int) * 256 * 256);
	for (int i = 0; i < 256 * 256; i++)
		n_data[i] = data[i * 4] % 256;
	return n_data;
}

unsigned char* int_array_to_picture(int* data)
{
	unsigned char* n_data = (unsigned char*)malloc(sizeof(unsigned char*) * 256 * 256 * 4);
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			if (data[i + j * 256] < 0)
				setPixel(n_data, i, j, 0);
			else if (data[i + j * 256] > 255)
				setPixel(n_data, i, j, 255);
			else
				setPixel(n_data, i, j, data[i + j * 256]);
	return n_data;
}

int* applyFilter(int* data, int filter[9], int filter_size)
{
	int* filtered_data = (int*)malloc(sizeof(int) * 256 * 256 * 4);
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
		{
			int filterSum = 0;
			for (int k = -1; k <= 1; k++)
				for (int l = -1; l <= 1; l++)
					if (i + k < 256 && i + k >= 0 && j + l < 256 && j + l >= 0)
						filterSum += getPixel(data, i + k, j + l) * filter[(k + 1) + 3 * (l + 1)];
			setPixel(filtered_data, i, j, (filterSum / filter_size));
		}
	return filtered_data;
}

void Game::Task4() {
	//read image
	int* isize_x = new int(256);
	int* isize_y = new int(256);
	int* c_num = new int(0);
	unsigned char* data = stbi_load("../res/textures/lena256.jpg", isize_x, isize_y, c_num, 4);
	int filter1[] = 
	{ 1,2,1,
	 2,4,2,
	 1,2,1 };
	int filter2[] =
	{ -1,0,1,
	 -2,0,2,
	 -1,0,1 };
	int filter3[] =
	{ 1,2,1,
	 0,0,0,
	 -1,-2,-1 };
	int filter4[] =
	{ 1,0,-1,
	 2,0,-2,
	 1,0,-1 };
	int filter5[] =
	{ -1,-2,-1,
	 0,0,0,
	 1,2,1 };

	//image permutation

	int* int_data = picture_to_int_array(data);
	int* s_data = applyFilter(int_data, filter1, 16);
	int* x_div = applyFilter(s_data, filter2, 1);
	int* y_div = applyFilter(s_data, filter3, 1);
	int* magnitude = (int*)malloc(sizeof(int) * 256 * 256);
	int* final_data = (int*)malloc(sizeof(int) * 256 * 256);
	double* angle = (double*)malloc(sizeof(double) * 256 * 256);
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
		{
			int x = (int)getPixel(x_div, i, j);
			int y = (int)getPixel(y_div, i, j);
			int temp = std::sqrt(x * x + y * y);
			setPixel(magnitude, i, j, temp);
			angle[i + j * 256] = std::atan2(y, x) * 180 / 3.14159265358;
		}
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
		{
			int direction = ((int)(angle[i + j * 256] + 22.5 + 180) / 45) % 4;
			int cur_pixel = getPixel(magnitude, i, j);
			if (i == 0 || j == 0 || i == 255 || j == 255)
				setPixel(final_data, i, j, 0);
			else if (direction == 0 && cur_pixel > getPixel(magnitude, i - 1, j) && cur_pixel > getPixel(magnitude, i + 1, j))
				setPixel(final_data, i, j, cur_pixel);
			else if (direction == 1 && cur_pixel > getPixel(magnitude, i + 1, j - 1) && cur_pixel > getPixel(magnitude, i - 1, j + 1))
				setPixel(final_data, i, j, cur_pixel);
			else if (direction == 2 && cur_pixel > getPixel(magnitude, i, j - 1) && cur_pixel > getPixel(magnitude, i, j + 1))
				setPixel(final_data, i, j, cur_pixel);
			else if (direction == 3 && cur_pixel > getPixel(magnitude, i - 1, j - 1) && cur_pixel > getPixel(magnitude, i + 1, j + 1))
				setPixel(final_data, i, j, cur_pixel);
			else
				setPixel(final_data, i, j, 0);
			if (getPixel(final_data, i, j) < 160)
				setPixel(final_data, i, j, 0);
		}

	unsigned char* final_picture = int_array_to_picture(final_data);

	delete data;
	delete int_data;
	delete s_data;
	delete x_div;
	delete y_div;
	delete magnitude;
	delete angle;
	delete final_data;

	//add image texture
	AddTexture(256, 256, final_picture);

	//save data
	WriteToTxt(final_picture, 256, 256, 1,"../assignment/img4.txt");
}
void Game::Task5() {
	// Read image
	int width, height, c_num;
	unsigned char* data = stbi_load("../res/textures/lena256.jpg", &width, &height, &c_num, 4);

	int white = 0;
	int black = 255;
	int level1 = (0.2 * 255);
	int level2 = (0.4 * 255);
	int level3 = (0.6 * 255);
	int level4 = (0.8 * 255);
	int greyScale = 4;
	int doubleWidth = 2 * width;
	int doubleHeight = 2 * height;
	unsigned char* image = (unsigned char*)(malloc(4 * doubleWidth * doubleHeight));

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			int idx = greyScale * (row * height + col);
			int val = data[idx]; // Assuming grayscale

			//halftone pattern
			unsigned char pattern[4];
			if (val < level1) {
				pattern[0] = pattern[1] = pattern[2] = pattern[3] = white;
			}
			else if (val < level2) {
				pattern[0] = pattern[1] = white;
				pattern[2] = black;
				pattern[3] = white;
			}
			else if (val < level3) {
				pattern[0] = white;
				pattern[1] = pattern[2] = black;
				pattern[3] = white;
			}
			else if (val < level4) {
				pattern[0] = white;
				pattern[1] = pattern[2] = pattern[3] = black;
			}
			else {
				pattern[0] = pattern[1] = pattern[2] = pattern[3] = black;
			}

			// Write the pattern to the image
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					int patternIdx = i * 2 + j;
					int imageIdx = greyScale * ((2 * row + i) * doubleWidth + (2 * col + j));
					image[imageIdx] = pattern[patternIdx];
					image[imageIdx + 1] = pattern[patternIdx];
					image[imageIdx + 2] = pattern[patternIdx];
					image[imageIdx + 3] = 255; // Alpha channel
				}
			}
		}
	}

	AddTexture(doubleWidth, doubleHeight, image);

	// Write the halftoned image to a text file
	WriteToTxt(image, 256, 256, 255, "../assignment/img5.txt");
	// Clean up
	free(image);
}