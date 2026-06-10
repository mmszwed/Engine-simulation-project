/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOśCI HANDLOWEJ albo PRZYDATNOśCI DO OKREśLONYCH
ZASTOSOWAń.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/


#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H


#include <GL/glew.h>
#include "stdio.h"


/**
 * @brief Loads, compiles, links and activates an OpenGL shader program.
 *
 * The loader validates file reads, shader compilation and program linking.
 * The project creates one material shader and one depth-only shadow shader.
 */
class ShaderProgram {
private:
	GLuint shaderProgram; //Uchwyt reprezentujący program cieniujacy
	GLuint vertexShader; //Uchwyt reprezentujący vertex shader
	GLuint geometryShader; //Uchwyt reprezentujący geometry shader
	GLuint fragmentShader; //Uchwyt reprezentujący fragment shader
	char* readFile(const char* fileName); //metoda wczytująca plik tekstowy do tablicy znaków
	GLuint loadShader(GLenum shaderType,const char* fileName); //Metoda wczytuje i kompiluje shader, a następnie zwraca jego uchwyt
public:
	/**
	 * @param vertexShaderFile Path to the vertex shader.
	 * @param geometryShaderFile Optional geometry shader path or nullptr.
	 * @param fragmentShaderFile Path to the fragment shader.
	 */
	ShaderProgram(const char* vertexShaderFile,const char* geometryShaderFile,const char* fragmentShaderFile);
	~ShaderProgram();

	/** @brief Activates the linked shader program. */
	void use();

	/** @return Location of a uniform variable in the linked program. */
	GLuint u(const char* variableName);

	/** @return Location of a vertex attribute in the linked program. */
	GLuint a(const char* variableName);
};



extern ShaderProgram* spEngine;
extern ShaderProgram* spShadow;

void initShaders();
void freeShaders();

#endif
