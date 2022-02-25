
/*************************************************************************************/

//  Szkielet programu do tworzenia modelu sceny 3-D z wizualizacj� osi
//  uk�adu wsp�rz�dnych dla rzutowania perspektywicznego

/*************************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

using namespace std;

#define M_PI 3.14159265358979323846

constexpr auto N = 60; //rozmier siatki


static GLfloat viewer[3] = { 10.0, 2.0, 0.0 };
static GLfloat points[2] = { 1.0 ,1.0 }; //tablica na k�ty theta i phi


static GLfloat x_angle;
static GLfloat y_angle;


static GLint status = 0;       // stan klawiszy myszy
  // 0 - nie naci�ni�to �adnego klawisza
  // 1 - naci�ni�ty zosta� lewy klawisz

static int x_pos_old = 0;       // poprzednia pozycja kursora myszy

static int y_pos_old = 0;       // poprzednia pozycja kursora myszy
static int R_pos_old = 0;       // poprzednia pozycja kursora myszy



static int delta_x = 0;        // r�nica pomi�dzy pozycj� bie��c�
 // i poprzedni� kursora myszy

static int delta_y = 0;        // r�nica pomi�dzy pozycj� bie��c�
 // i poprzedni� kursora myszy

static int delta_R = 0;        // r�nica pomi�dzy pozycj� bie��c�
 // i poprzedni� kursora myszy


static GLfloat startRadius = 10.0;
static GLfloat startU = 1.0;
static GLfloat startV = 1.0;
static GLfloat theta[2] = { 5.0 , 5.0 };
static GLfloat fi[2] = { 5.0 , 1.0 };
int model = 1;
bool changeTriangle[5];
bool drawEgg = true;
bool drawTriangle = false;


/*************************************************************************************/

GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat)
{

	/*************************************************************************************/

	// Struktura dla nag��wka pliku  TGA


#pragma pack(1)          
	typedef struct
	{
		GLbyte    idlength;
		GLbyte    colormaptype;
		GLbyte    datatypecode;
		unsigned short    colormapstart;
		unsigned short    colormaplength;
		unsigned char     colormapdepth;
		unsigned short    x_orgin;
		unsigned short    y_orgin;
		unsigned short    width;
		unsigned short    height;
		GLbyte    bitsperpixel;
		GLbyte    descriptor;
	}TGAHEADER;
#pragma pack(8)

	FILE* pFile;
	TGAHEADER tgaHeader;
	unsigned long lImageSize;
	short sDepth;
	GLbyte* pbitsperpixel = NULL;


	/*************************************************************************************/

	// Warto�ci domy�lne zwracane w przypadku b��du

	*ImWidth = 0;
	*ImHeight = 0;
	*ImFormat = GL_BGR_EXT;
	*ImComponents = GL_RGB8;

	pFile = fopen(FileName, "rb");
	if (pFile == NULL)
		return NULL;

	/*************************************************************************************/
	// Przeczytanie nag��wka pliku


	fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);


	/*************************************************************************************/

	// Odczytanie szeroko�ci, wysoko�ci i g��bi obrazu

	*ImWidth = tgaHeader.width;
	*ImHeight = tgaHeader.height;
	sDepth = tgaHeader.bitsperpixel / 8;


	/*************************************************************************************/
	// Sprawdzenie, czy g��bia spe�nia za�o�one warunki (8, 24, lub 32 bity)

	if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
		return NULL;

	/*************************************************************************************/

	// Obliczenie rozmiaru bufora w pami�ci


	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;


	/*************************************************************************************/

	// Alokacja pami�ci dla danych obrazu


	pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

	if (pbitsperpixel == NULL)
		return NULL;

	if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
	{
		free(pbitsperpixel);
		return NULL;
	}


	/*************************************************************************************/

	// Ustawienie formatu OpenGL


	switch (sDepth)

	{

	case 3:

		*ImFormat = GL_BGR_EXT;

		*ImComponents = GL_RGB8;

		break;

	case 4:

		*ImFormat = GL_BGRA_EXT;

		*ImComponents = GL_RGBA8;

		break;

	case 1:

		*ImFormat = GL_LUMINANCE;

		*ImComponents = GL_LUMINANCE8;

		break;

	};



	fclose(pFile);



	return pbitsperpixel;

}

void Mouse(int btn, int state, int x, int y)
{


	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		x_pos_old = x;        // przypisanie aktualnie odczytanej pozycji kursora
		// jako pozycji poprzedniej
		y_pos_old = y;

		status = 1;          // wci�ni�ty zosta� lewy klawisz myszy
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		x_pos_old = x;
		y_pos_old = y;
		R_pos_old = y;
		status = 2; // wci�ni�ty zosta� prawy klawisz myszy

	}
	else

		status = 0;          // nie zosta� wci�ni�ty �aden klawisz
}


void Motion(GLsizei x, GLsizei y)
{

	delta_x = x - x_pos_old;     // obliczenie r�nicy po�o�enia kursora myszy
	delta_y = y - y_pos_old;
	delta_R = y - y_pos_old;
	y_pos_old = y;
	x_pos_old = x;            // podstawienie bie��cego po�o�enia jako poprzednie
	R_pos_old = y;

	glutPostRedisplay();     // przerysowanie obrazu sceny
}

//obliczenie zmiennej x dla wektora normalnego
GLfloat getNormalizedX(float u, float v)
{
	float yu, yv, zu, zv, alfa;
	alfa = v * M_PI;
	yu = 640 * pow(u, 3) - 960 * pow(u, 2) + 320 * u;
	yv = 0;
	zu = (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u - 45) * sin(alfa);
	zv = -M_PI * (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * pow(u, 3) - 180 * pow(u, 2) + 45 * u) *
		cos(alfa);
	return (GLfloat)(yu * zv - zu * yv);

}

//obliczenie zmiennej y dla wektora normalnego
GLfloat getNormalizedY(float u, float v)
{
	float xu, xv, zu, zv, alfa;
	alfa = v * M_PI;
	xu = (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u - 45) * cos(alfa);
	xv = M_PI * (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * pow(u, 3) - 180 * pow(u, 2) + 45 * u) *
		sin(alfa);
	zu = (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u - 45) * sin(alfa);
	zv = -M_PI * (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * pow(u, 3) - 180 * pow(u, 2) + 45 * u) *
		cos(alfa);
	return (GLfloat)(zu * xv - xu * zv);

}

//obliczenie zmiennej z dla wektora normalnego
GLfloat getNormalizedZ(float u, float v)
{
	float xu, xv, yv, yu, alfa;
	alfa = v * M_PI;
	xu = (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u - 45) * cos(alfa);
	xv = M_PI * (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * pow(u, 3) - 180 * pow(u, 2) + 45 * u) *
		sin(alfa);
	yu = 640 * pow(u, 3) - 960 * pow(u, 2) + 320 * u;
	yv = 0;
	return (GLfloat)(xu * yv - yu * xv);

}

void Egg()
{
	//zmienne u i v dziedziny parametrycznej
	float u;
	float v;
	float array[N][N][3]; //tablica na wsp�rz�dne x,y,z
	GLfloat x, y, z;
	GLfloat arrayForLight[N][N][3]; //tablica na wetkory normalne
	float text[N][N][2]; //tablica na wsp�rz�dne x,y,z


	for (int i = 0; i < N; i++)
	{
		u = (float)i / (N - 1); //na�o�enie siatki

		for (int j = 0; j < N; j++)
		{
			v = (float)j / (N - 1); //na�o�enie siatki
			text[i][j][0] = u;
			text[i][j][1] = v;

			//wsp�rz�dna x
			array[i][j][0] = (-90 * pow(u, 5) + 225 * pow(u, 4) - 270 * pow(u, 3) + 180 * pow(u, 2) - 45 * u) * (cos(M_PI * v));
			//wsp�rz�dna y
			array[i][j][1] = (160 * pow(u, 4) - 320 * pow(u, 3) + 160 * pow(u, 2));
			//wsp�rz�dna z
			array[i][j][2] = (-90 * pow(u, 5) + 225 * pow(u, 4) - 270 * pow(u, 3) + 180 * pow(u, 2) - 45 * u) * (sin(M_PI * v));

			//obliczenie zmiennych wektroa normalnego
			x = getNormalizedX(u, v);
			y = getNormalizedY(u, v);
			z = getNormalizedZ(u, v);

			//normalizacja dla r�nych mo�liwo�ci po�o�enia punktu
			if (i < N / 2)
			{
				arrayForLight[i][j][0] = x / (float)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
				arrayForLight[i][j][1] = y / (float)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
				arrayForLight[i][j][2] = z / (float)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
			}

			if (i > N / 2)
			{
				arrayForLight[i][j][0] = -1.0 * x / (float)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
				arrayForLight[i][j][1] = -1.0 * y / (float)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
				arrayForLight[i][j][2] = -1.0 * z / (float)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
			}
			if (i == N / 2)
			{
				arrayForLight[i][j][0] = 0;
				arrayForLight[i][j][1] = 1;
				arrayForLight[i][j][2] = 0;
			}
			if (i == 0 || i == N - 1)
			{
				arrayForLight[i][j][0] = 0;
				arrayForLight[i][j][1] = -1;
				arrayForLight[i][j][2] = 0;
			}

		}
	}

	glTranslated(0.0, -5.0, 0.0); //rotacja wzgl�dem osi OX

	//rysowanie jajka, zbudowanego z tr�jk�t�w

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N - 1; j++)
		{
		
			//funckja modul N jest u�yta w celu zabezpieczenia przed wyj�ciem poza zakres tablic
			//jednocze�nie ��czy ostatni punkt pierwszym
			
			glBegin(GL_TRIANGLES);
			
			glNormal3fv(arrayForLight[i][j]);
			glTexCoord2fv(text[i][j]);
			glVertex3fv(array[i][j]);


			
			glNormal3fv(arrayForLight[i][(j + 1) % N]);
			glTexCoord2fv(text[i][(j + 1) % N]);
			glVertex3fv(array[i][(j + 1) % N]);

		

			
			glNormal3fv(arrayForLight[(i + 1) % N][j]);
			glTexCoord2fv(text[(i + 1) % N][j]);
			glVertex3fv(array[(i + 1) % N][j]);


			glEnd();

			glBegin(GL_TRIANGLES);
		
			glNormal3fv(arrayForLight[i][j]);
			glTexCoord2fv(text[i][j]);
			glVertex3fv(array[i][j]);

			
			glNormal3fv(arrayForLight[(i + 1) % N][j]);
			glTexCoord2fv(text[(i + 1) % N][j]);
			glVertex3fv(array[(i + 1) % N][j]);

		
			glNormal3fv(arrayForLight[i][(j + 1) % N]);
			glTexCoord2fv(text[i][(j + 1) % N]);
			glVertex3fv(array[i][(j + 1) % N]);


			glEnd();
		

			glBegin(GL_TRIANGLES);

			
	
			glNormal3fv(arrayForLight[(i + 1) % N][j]);
			glTexCoord2fv(text[(i + 1) % N][j]);
			glVertex3fv(array[(i + 1) % N][j]);


			glNormal3fv(arrayForLight[i][(j + 1) % N]);
			glTexCoord2fv(text[i][(j + 1) % N]);
			glVertex3fv(array[i][(j + 1) % N]);


			glNormal3fv(arrayForLight[(i + 1) % N][(j + 1) % N]);
			glTexCoord2fv(text[(i + 1) % N][(j + 1) % N]);
			glVertex3fv(array[(i + 1) % N][(j + 1) % N]);


			glEnd();

			glBegin(GL_TRIANGLES);



			glNormal3fv(arrayForLight[(i + 1) % N][j]);
			glTexCoord2fv(text[(i + 1) % N][j]);
			glVertex3fv(array[(i + 1) % N][j]);

			glNormal3fv(arrayForLight[(i + 1) % N][(j + 1) % N]);
			glTexCoord2fv(text[(i + 1) % N][(j + 1) % N]);
			glVertex3fv(array[(i + 1) % N][(j + 1) % N]);

			glNormal3fv(arrayForLight[i][(j + 1) % N]);
			glTexCoord2fv(text[i][(j + 1) % N]);
			glVertex3fv(array[i][(j + 1) % N]);



			glEnd();

		}


}


/*************************************************************************************/

// Funkcja okre�laj�ca co ma by� rysowane (zawsze wywo�ywana, gdy trzeba
// przerysowa� scen�)
// 
//rysowanie 5 �cian piramidy w odpowiedniej kolejno�ci
void triangle()
{
	glBegin(GL_TRIANGLES);

	//1 �ciana

	if (changeTriangle[0]) {
		glNormal3f(0.0f, 3.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 3.0f, 0.0f);

		glNormal3f(-3.0f, 0.0f, 3.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-3.0f, 0.0f, 3.0f);


		glNormal3f(3.0f, 0.0f, 3.0f);
		glTexCoord2f(0.5f, 1.0f);
		glVertex3f(3.0f, 0.0f, 3.0f);
	}

	//2 �ciana
	if (changeTriangle[1]) {

		glNormal3f(0.0f, 3.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 3.0f, 0.0f);

		glNormal3f(3.0f, 0.0f, 3.0f);
		glTexCoord2f(0.5f, 1.0f);
		glVertex3f(3.0f, 0.0f, 3.0f);

		glNormal3f(3.0f, 0.0f, -3.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(3.0f, 0.0f, -3.0f);
	}

	//3 �ciana
	if (changeTriangle[2]) {
		glNormal3f(0.0f, 3.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 3.0f, 0.0f);

		glNormal3f(3.0f, 0.0f, -3.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(3.0f, 0.0f, -3.0f);

		glNormal3f(-3.0f, 0.0f, -3.0f);
		glTexCoord2f(0.5f, 1.0f);
		glVertex3f(-3.0f, 0.0f, -3.0f);

	}

	//4 �ciana
	if (changeTriangle[3]) {

		glNormal3f(0.0f, 3.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 3.0f, 0.0f);

		glNormal3f(-3.0f, 0.0f, -3.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-3.0f, 0.0f, -3.0f);

		glNormal3f(-3.0f, 0.0f, 3.0f);
		glTexCoord2f(0.5f, 1.0f);
		glVertex3f(-3.0f, 0.0f, 3.0f);
	}


	glEnd();

	glBegin(GL_QUADS);

	// 5 �ciana
	if (changeTriangle[4]) {

		glNormal3f(3.0f, 0.0f, 3.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(3.0f, 0.0f, 3.0f);

		glNormal3f(-3.0f, 0.0f, 3.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-3.0f, 0.0f, 3.0f);

		glNormal3f(-3.0f, 0.0f, -3.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-3.0f, 0.0f, -3.0f);

		glNormal3f(3.0f, 0.0f, -3.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(3.0f, 0.0f, -3.0f);
	}
	glEnd();

}



void RenderScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Czyszczenie okna aktualnym kolorem czyszcz�cym

	glLoadIdentity();
	// Czyszczenie macierzy bie??cej


	// Narysowanie osi przy pomocy funkcji zdefiniowanej powy�ej
	if (model == 1) {


		glDisable(GL_LIGHT2); //wy��czenie �wiat�a z drugiego modelu
		glEnable(GL_LIGHT0); //w��czenie �wiate�
		glEnable(GL_LIGHT1);


		gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, cos(points[1]), 0.0);
		if (status == 1)                     // je�li lewy klawisz myszy wci�ni�ty
		{
			//obliczenie k�ta theta i na�o�enie ogranicze� niepozwalaj�cych przekroczy� zakresu od 0 do 2PI
			theta[0] -= (float)delta_x * x_angle;

			if (theta[0] >= 2 * M_PI)
				theta[0] -= 2 * M_PI;

			if (theta[0] <= 0)
				theta[0] += 2 * M_PI;

			//obliczenie k�ta phi i na�o�enie ogranicze� niepozwalaj�cych przekroczy� zakresu od 0 do 2PI
			fi[0] -= (float)delta_y * y_angle;

			if (fi[0] >= 2 * M_PI)
				fi[0] -= 2 * M_PI;

			if (fi[0] <= 0)
				fi[0] += 2 * M_PI;


		}
		else if (status == 2) // je�li prawy klawisz myszy wci�ni�ty
		{
			//obliczenie k�ta theta i na�o�enie ogranicze� niepozwalaj�cych przekroczy� zakresu od 0 do 2PI
			theta[1] -= (float)delta_x * x_angle;

			if (theta[1] >= 2 * M_PI)
				theta[1] -= 2 * M_PI;

			if (theta[1] <= 0)
				theta[1] += 2 * M_PI;

			//obliczenie k�ta phi i na�o�enie ogranicze� niepozwalaj�cych przekroczy� zakresu od 0 do 2PI
			fi[1] -= (float)delta_y * y_angle;

			if (fi[1] >= 2 * M_PI)
				fi[1] -= 2 * M_PI;

			if (fi[1] <= 0)
				fi[1] += 2 * M_PI;

		}

	}

	if (model == 2) // jajko jednego kolory
	{
		glEnable(GL_LIGHT2);
		glDisable(GL_LIGHT0); //wy��czenie �wiate�
		glDisable(GL_LIGHT1);


		gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, cos(points[1]), 0.0);

	}
	if (model == 3)
	{
		{
			if (status == 1)                     // je�li lewy klawisz myszy wci�ni�ty
			{
				// obliczenie theta, �e k�t b�dzie mniejszy od 2PI
				points[0] += delta_x * x_angle;
				if (points[0] >= 360.0)
					points[0] = 0.0;

				// obliczenie phi, zapewniene, �e k�t b�dzie mniejszy od 2PI
				points[1] += delta_y * y_angle;
				if (points[1] >= 360.0)
					points[1] = 0.0;

			}
			// je�li prawy klawisz myszy wci�ni�ty
			else if (status == 2)
			{
				startRadius += delta_R * y_angle;
			}

			//obliczenie punkt�w wg wzor�w
			viewer[0] = startRadius * cos(points[0]) * cos(points[1]);
			viewer[1] = startRadius * sin(points[1]);
			viewer[2] = startRadius * sin(points[0]) * cos(points[1]);

			//zmiana po�o�enia obserwatora
			gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, cos(points[1]), 0.0);


		}

	}

	GLfloat light[4];
	light[0] = startRadius * cos(theta[0]) * cos(fi[0]);
	light[1] = startRadius * sin(fi[0]);
	light[2] = startRadius * sin(theta[0]) * cos(fi[0]);
	light[3] = 1.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light);


	light[0] = startRadius * cos(theta[1]) * cos(fi[1]);
	light[1] = startRadius * sin(fi[1]);
	light[2] = startRadius * sin(theta[1]) * cos(fi[1]);
	light[3] = 1.0;
	glLightfv(GL_LIGHT1, GL_POSITION, light);


	if(drawEgg)
	Egg();

	if (drawTriangle)
		triangle();



	glFlush();
	// Przekazanie polece� rysuj�cych do wykonania

	glutSwapBuffers();



}
/*************************************************************************************/

// Funkcja ustalaj�ca stan renderowania


void keys(unsigned char key, int x, int y)
{
	if (key == '1')
		model = 1;

	if (key == '2')
		model = 2;

	if (key == '3')
		model = 3;
	if (key == '4') {
		drawEgg = true;
		drawTriangle = false;
	}
	if (key == '5') {
		drawEgg = false;
		drawTriangle = true;
	}

	if (key == '6')
		if (!changeTriangle[1])
			changeTriangle[1] = true;
		else
			changeTriangle[1] = false;
	if (key == '7')
		if (!changeTriangle[2])
			changeTriangle[2] = true;
		else
			changeTriangle[2] = false;
	if (key == '8')
		if (!changeTriangle[3])
			changeTriangle[3] = true;
		else
			changeTriangle[3] = false;
	if (key == '0')
		if (!changeTriangle[4])
			changeTriangle[4] = true;
		else
			changeTriangle[4] = false;

	if (key == '9')
		if (!changeTriangle[0])
			changeTriangle[0] = true;
		else
			changeTriangle[0] = false;

	RenderScene(); // przerysowanie obrazu sceny
}
void MyInit(void)
{

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszcz�cy (wype�nienia okna) ustawiono na czarny
	// Definicja materia�u z jakiego zrobiony jest czajnik

	// Zmienne dla obrazu tekstury


	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;

	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki ka =[kar,kag,kab] dla �wiat�a otoczenia

	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki kd =[kdr,kdg,kdb] �wiat�a rozproszonego

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki ks =[ksr,ksg,ksb] dla �wiat�a odbitego              

	GLfloat mat_shininess = { 20.0 };
	// wsp�czynnik n opisuj�cy po�ysk powierzchni

	/*************************************************************************************/
	// Definicja �r�d�a �wiat�a

	GLfloat light_position1[] = { 0.0, 0.0, 10.0, 1.0 };
	GLfloat light_position2[] = { 0.0, 0.0, 10.0, 1.0 };
	GLfloat light_position3[] = { 0.0, 0.0, 10.0, 1.0 };

	// po�o�enie �r�d�a


	GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a otoczenia
	// Ia = [Iar,Iag,Iab]

	GLfloat light_diffuse1[] = { 0.0, 0.0, 1.0, 0.0 };
	GLfloat light_diffuse2[] = { 0.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse3[] = { 5.0, 6.0, 1.0, 3.0 };

	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a powoduj�cego
	// odbicie dyfuzyjne Id = [Idr,Idg,Idb]

	GLfloat light_specular1[] = { 0.0, 0.0, 1.0, 0.0 };
	GLfloat light_specular2[] = { 0.0, 0.4, 0.0, 1.0 };
	GLfloat light_specular3[] = { 0.0, 5.0, 0.0, 1.0 };


	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a powoduj�cego
	// odbicie kierunkowe Is = [Isr,Isg,Isb]

	GLfloat att_constant = { 1.0 };
	// sk�adowa sta�a ds dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	GLfloat att_linear = { 0.05 };
	// sk�adowa liniowa dl dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	GLfloat att_quadratic = { 0.001 };
	// sk�adowa kwadratowa dq dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	/*************************************************************************************/
	// Ustawienie parametr�w materia�u i �r�d�a �wiat�a

	/*************************************************************************************/
	// Ustawienie patrametr�w materia�u


	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	/*************************************************************************************/
	// Ustawienie parametr�w �r�d�a

	//ustawienia 1 �wiat�a

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular1);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);

	//ustawienia 2 �wiat�a

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular2);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, att_quadratic);

	//ustawienia 3 �wiat�a

	glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse3);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular3);
	glLightfv(GL_LIGHT2, GL_POSITION, light_position3);
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, att_quadratic);

	/*************************************************************************************/
	// Ustawienie opcji systemu o�wietlania sceny
	glShadeModel(GL_SMOOTH); // w�aczenie �agodnego cieniowania
	glEnable(GL_DEPTH_TEST); // w��czenie mechanizmu z-bufora
	glEnable(GL_LIGHTING);   // w�aczenie systemu o�wietlenia sceny

	/*************************************************************************************/
	// Teksturowanie b�dzie prowadzone tyko po jednej stronie �ciany

	glEnable(GL_CULL_FACE);
	/*************************************************************************************/

	//  Przeczytanie obrazu tekstury z pliku o nazwie tekstura.tga

	pBytes = LoadTGAImage("M1_t.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);

	/*************************************************************************************/

	   // Zdefiniowanie tekstury 2-D

	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

	/*************************************************************************************/

	// Zwolnienie pami�ci

	free(pBytes);

	/*************************************************************************************/

	// W��czenie mechanizmu teksturowania

	glEnable(GL_TEXTURE_2D);

	/*************************************************************************************/

	// Ustalenie trybu teksturowania

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/*************************************************************************************/

	// Okre�lenie sposobu nak�adania tekstur

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie sta�ych proporcji rysowanych
// w przypadku zmiany rozmiar�w okna.
// Parametry vertical i horizontal (wysoko�� i szeroko�� okna) s�
// przekazywane do funkcji za ka�dym razem gdy zmieni si� rozmiar okna.



void ChangeSize(GLsizei horizontal, GLsizei vertical)
{


	x_angle = 360.0 * 0.05 / (float)horizontal; //0,05 dla spowolnienia ruch�w myszy
	y_angle = 360.0 * 0.05 / (float)vertical;

	glMatrixMode(GL_PROJECTION);
	// Prze��czenie macierzy bie��cej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bie��cej

	gluPerspective(70, 1.0, 1.0, 30.0);
	// Ustawienie parametr�w dla rzutu perspektywicznego


	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
	// Ustawienie wielko�ci okna okna widoku (viewport) w zale�no�ci
	// relacji pomi�dzy wysoko�ci� i szeroko�ci� okna

	glMatrixMode(GL_MODELVIEW);
	// Prze��czenie macierzy bie��cej na macierz widoku modelu  

	glLoadIdentity();
	// Czyszczenie macierzy bie��cej

}

/*************************************************************************************/

// G��wny punkt wej�cia programu. Program dzia�a w trybie konsoli



void main(void)
{


	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(300, 300);
	for (int i = 0; i < 5; i++)
		changeTriangle[i] = true;

	glutCreateWindow("Sterowanie swiatlem");
	cout << "Sterowanie progrem odbywa sie za pomoca przyciskow klawaiatury oraz myszy:" << endl;
	cout << " 1 - dwa zrodla swiatla" << endl;
	cout << " 2 - jedno zrodlo swiatla" << endl;
	cout << " 3 - zmiana punktu obesrwacji " << endl;
	cout << " 4- rysowanie jajka" << endl;
	cout << " 5- rysowanie piramidy" << endl;
	cout << " Przyciski 6-0 wlaczaja/wylaczaja sciany piramidy" << endl;

	glutDisplayFunc(RenderScene);
	// Okre�lenie, �e funkcja RenderScene b�dzie funkcj� zwrotn�
	// (callback function).  B�dzie ona wywo�ywana za ka�dym razem
	// gdy zajdzie potrzeba przerysowania okna

	glutMouseFunc(Mouse);
	// Ustala funkcj� zwrotn� odpowiedzialn� za badanie stanu myszy

	glutMotionFunc(Motion);
	// Ustala funkcj� zwrotn� odpowiedzialn� za badanie ruchu myszy
	glutKeyboardFunc(keys);
	glutReshapeFunc(ChangeSize);
	// Dla aktualnego okna ustala funkcj� zwrotn� odpowiedzialn�
	// za zmiany rozmiaru okna                      



	MyInit();
	// Funkcja MyInit() (zdefiniowana powy�ej) wykonuje wszelkie
	// inicjalizacje konieczne  przed przyst�pieniem do renderowania

	glEnable(GL_DEPTH_TEST);
	// W��czenie mechanizmu usuwania niewidocznych element�w sceny

	glutMainLoop();
	// Funkcja uruchamia szkielet biblioteki GLUT

}

/*************************************************************************************/