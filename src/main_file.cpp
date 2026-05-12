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

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

GLuint vao = 0;
float speed = 3.14f; // [radiany/s]

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	glEnable(GL_DEPTH_TEST);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************	
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************	
}



//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle) {
	//************Tutaj umieszczaj kod rysujący obraz******************
	glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	glm::mat4 M=glm::mat4(1.0f);
	M=glm::rotate(M, angle, glm::vec3(0.0f, 1.0f, 0.0f));
	M=glm::rotate(M, 72.0f*PI/180.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 V=glm::lookAt(
 		glm::vec3(0.0f,0.0f,-5.0f),
  		glm::vec3(0.0f,0.0f,0.0f),
    	glm::vec3(0.0f,1.0f,0.0f)
	);

	glm::mat4 P=glm::perspective(50.0f*PI/180.f,1.0f,1.0f,50.0f);

	spConstant->use();//Aktywacja programu cieniującego
	glUniformMatrix4fv(spConstant->u("P"),1,false,glm::value_ptr(P));
	glUniformMatrix4fv(spConstant->u("V"),1,false,glm::value_ptr(V));
	glUniformMatrix4fv(spConstant->u("M"),1,false,glm::value_ptr(M));

	Models::torus.drawWire();

	glfwSwapBuffers(window);
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // wymagane na macOS

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}
	glGetError();

	initOpenGLProgram(window); //Operacje inicjujące

	float angle = 0;
	glfwSetTime(0);
	//Główna pętla	
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{		
		angle += speed * glfwGetTime();
		glfwSetTime(0);
		drawScene(window, angle); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
