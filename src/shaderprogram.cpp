/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#include "shaderprogram.h"


ShaderProgram* spEngine;
ShaderProgram* spShadow;


void initShaders() {
	spEngine = new ShaderProgram("shaders/v_engine.glsl", NULL, "shaders/f_engine.glsl");
	spShadow = new ShaderProgram("shaders/v_shadow.glsl", NULL, "shaders/f_shadow.glsl");
}

void freeShaders() {
	delete spEngine;
	delete spShadow;
}

//Procedura wczytuje plik do tablicy znaków.
char* ShaderProgram::readFile(const char* fileName) {
	FILE* file = nullptr;
#ifdef _MSC_VER
	if (fopen_s(&file, fileName, "rb") != 0) {
		file = nullptr;
	}
#else
	file = fopen(fileName, "rb");
#endif

	if (file == nullptr) {
		fprintf(stderr, "Nie mozna otworzyc pliku shadera: %s\n", fileName);
		return nullptr;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		fprintf(stderr, "Nie mozna odczytac rozmiaru shadera: %s\n", fileName);
		fclose(file);
		return nullptr;
	}

	const long fileSize = ftell(file);
	if (fileSize < 0 || fseek(file, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Nieprawidlowy plik shadera: %s\n", fileName);
		fclose(file);
		return nullptr;
	}

	char* result = new char[static_cast<size_t>(fileSize) + 1];
	const size_t expectedSize = static_cast<size_t>(fileSize);
	const size_t readSize = fread(result, 1, expectedSize, file);
	fclose(file);

	if (readSize != expectedSize) {
		fprintf(stderr, "Nie udalo sie wczytac calego shadera: %s\n", fileName);
		delete[] result;
		return nullptr;
	}

	result[readSize] = '\0';
	return result;
}

//Metoda wczytuje i kompiluje shader, a następnie zwraca jego uchwyt
GLuint ShaderProgram::loadShader(GLenum shaderType,const char* fileName) {
	//Wygeneruj uchwyt na shader
	GLuint shader=glCreateShader(shaderType);//shaderType to GL_VERTEX_SHADER, GL_GEOMETRY_SHADER lub GL_FRAGMENT_SHADER
	//Wczytaj plik ze źródłem shadera do tablicy znaków
	const GLchar* shaderSource=readFile(fileName);
	if (shaderSource == nullptr) {
		glDeleteShader(shader);
		return 0;
	}
	//Powiąż źródło z uchwytem shadera
	glShaderSource(shader,1,&shaderSource,NULL);
	//Skompiluj źródło
	glCompileShader(shader);
	//Usuń źródło shadera z pamięci (nie będzie już potrzebne)
	delete []shaderSource;

	//Pobierz log błędów kompilacji i wyświetl
	int infologLength = 0;
	int charsWritten  = 0;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1) {
		char* infoLog = new char[infologLength];
		glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "Shader %s:\n%s\n", fileName, infoLog);
		delete []infoLog;
	}

	GLint compileStatus = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus != GL_TRUE) {
		glDeleteShader(shader);
		return 0;
	}

	//Zwróć uchwyt wygenerowanego shadera
	return shader;
}

ShaderProgram::ShaderProgram(const char* vertexShaderFile,const char* geometryShaderFile,const char* fragmentShaderFile) {
	//Wczytaj vertex shader
	printf("Loading vertex shader...\n");
	vertexShader=loadShader(GL_VERTEX_SHADER,vertexShaderFile);

	//Wczytaj geometry shader
	if (geometryShaderFile!=NULL) {
		printf("Loading geometry shader...\n");
		geometryShader=loadShader(GL_GEOMETRY_SHADER,geometryShaderFile);
	} else {
		geometryShader=0;
	}

	//Wczytaj fragment shader
	printf("Loading fragment shader...\n");
	fragmentShader=loadShader(GL_FRAGMENT_SHADER,fragmentShaderFile);

	if (vertexShader == 0 || fragmentShader == 0 || (geometryShaderFile != NULL && geometryShader == 0)) {
		fprintf(stderr, "Nie mozna utworzyc programu shaderow.\n");
		shaderProgram = 0;
		return;
	}

	//Wygeneruj uchwyt programu cieniującego
	shaderProgram=glCreateProgram();

	//Podłącz do niego shadery i zlinkuj program
	glAttachShader(shaderProgram,vertexShader);
	glAttachShader(shaderProgram,fragmentShader);
	if (geometryShaderFile!=NULL) glAttachShader(shaderProgram,geometryShader);
	glLinkProgram(shaderProgram);

	//Pobierz log błędów linkowania i wyświetl
	int infologLength = 0;
	int charsWritten  = 0;

	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 1)
	{
		char* infoLog = new char[infologLength];
		glGetProgramInfoLog(shaderProgram, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "Program shaderow:\n%s\n", infoLog);
		delete []infoLog;
	}

	GLint linkStatus = GL_FALSE;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
		return;
	}

	printf("Shader program created \n");
}

ShaderProgram::~ShaderProgram() {
	if (shaderProgram == 0) {
		if (vertexShader != 0) glDeleteShader(vertexShader);
		if (geometryShader != 0) glDeleteShader(geometryShader);
		if (fragmentShader != 0) glDeleteShader(fragmentShader);
		return;
	}

	//Odłącz shadery od programu
	glDetachShader(shaderProgram, vertexShader);
	if (geometryShader!=0) glDetachShader(shaderProgram, geometryShader);
	glDetachShader(shaderProgram, fragmentShader);

	//Wykasuj shadery
	glDeleteShader(vertexShader);
	if (geometryShader!=0) glDeleteShader(geometryShader);
	glDeleteShader(fragmentShader);

	//Wykasuj program
	glDeleteProgram(shaderProgram);
}


//Włącz używanie programu cieniującego reprezentowanego przez aktualny obiekt
void ShaderProgram::use() {
	if (shaderProgram != 0) {
		glUseProgram(shaderProgram);
	}
}

//Pobierz numer slotu odpowiadającego zmiennej jednorodnej o nazwie variableName
GLuint ShaderProgram::u(const char* variableName) {
	return glGetUniformLocation(shaderProgram,variableName);
}

//Pobierz numer slotu odpowiadającego atrybutowi o nazwie variableName
GLuint ShaderProgram::a(const char* variableName) {
	return glGetAttribLocation(shaderProgram,variableName);
}
