/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 01/03/2023
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

//RAPID JSON
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include "Shader.h"

#include "Object.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	//glm::vec3 normal;
};


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// Protótipo da função de callback do mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
;

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX=false, rotateY=false, rotateZ=false;

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

int selected = 0;

double axisX = 1.0;
double axisY = 1.0;
double axisZ = 1.0;

bool firstMouse = true;
float lastX, lastY;
float sensitivity = 0.05;
float pitch = 0.0, yaw = -90.0;
float fov = 45.0;

vector <Object> objects;

// Função MAIN
int main()
{	
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	

	glfwSetCursorPos(window,WIDTH / 2, HEIGHT / 2);


	//Desabilita o desenho do cursor 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	//GLuint shader.ID = setupShader();
	Shader shader("Phong.vs","Phong.fs");

	glUseProgram(shader.ID);

	//Matriz de view -- posição e orientação da câmera
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("view",value_ptr(view));

	//Matriz de projeção perspectiva - definindo o volume de visualização (frustum)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));

	glEnable(GL_DEPTH_TEST);

	//vector <Object> objects = readConfigurationFile("../../config.json", &shader);
	ifstream in("../../config.json");
	string contents((std::istreambuf_iterator<char>(in)),
		istreambuf_iterator<char>());

	rapidjson::Document document;
	document.Parse(contents.c_str());

	glm::vec3 lightPos;
	glm::vec3 lightColor;

	for (int i = 0; i < document["objects"].Size(); i++) {
		Object obj;
		//Camera
		//Posicao
		cameraPos = glm::vec3(static_cast<double>(document["camera"]["position"]["x"].GetFloat()),
								static_cast<double>(document["camera"]["position"]["y"].GetFloat()),
								static_cast<double>(document["camera"]["position"]["z"].GetFloat()));
		
		//Front
		cameraFront = glm::vec3(static_cast<double>(document["camera"]["front"]["x"].GetFloat()),
								static_cast<double>(document["camera"]["front"]["y"].GetFloat()),
								static_cast<double>(document["camera"]["front"]["z"].GetFloat()));
		
		//Up
		cameraUp = glm::vec3(static_cast<double>(document["camera"]["up"]["x"].GetFloat()),
							static_cast<double>(document["camera"]["up"]["y"].GetFloat()),
							static_cast<double>(document["camera"]["up"]["z"].GetFloat()));

		//FOV
		fov = document["camera"]["fov"].GetFloat();
		

		//Modelos
		//Coletar diretório
		string folder = document["objects"][i]["folder"].GetString();

		//Coletar modelo
		string objfile = document["objects"][i]["model"].GetString();

		//Coletar mtl
		string mtl = document["objects"][i]["mtl"].GetString();

		//Coletar posição
		float x = document["objects"][i]["position"]["x"].GetFloat();
		float y = document["objects"][i]["position"]["y"].GetFloat();
		float z = document["objects"][i]["position"]["z"].GetFloat();
		glm::vec3 position = glm::vec3(x, y, z);

		//Coletar escala
		glm::vec3 scale = glm::vec3(document["objects"][i]["scale"]["x"].GetFloat(),
									document["objects"][i]["scale"]["y"].GetFloat(),
									document["objects"][i]["scale"]["z"].GetFloat());

		//Coletar eixo
		glm::vec3 axis = glm::vec3(document["objects"][i]["axis"]["x"].GetFloat(),
									document["objects"][i]["axis"]["y"].GetFloat(),
									document["objects"][i]["axis"]["z"].GetFloat());

		//Coletar angulo
		float angle = document["objects"][i]["angle"].GetFloat();
		

		//Inicializar objeto
		obj.initialize(folder, objfile, mtl, &shader, position, scale, angle, axis);
		objects.push_back(obj);

		//Coletar luz
		//Posicao
		lightPos = glm::vec3(document["light"]["position"]["x"].GetFloat(),
							document["light"]["position"]["y"].GetFloat(),
							document["light"]["position"]["y"].GetFloat());
		//Cor
		lightColor = glm::vec3(document["light"]["color"]["r"].GetFloat(),
								document["light"]["color"]["g"].GetFloat(),
								document["light"]["color"]["b"].GetFloat());
		//Expoente de Phong
		float q = document["light"]["q"].GetFloat();
		shader.setFloat("q", q);
	}

	

	//Definindo as propriedades do material da superficie
	//shader.setFloat("ka", 0.4);
	//shader.setFloat("kd", 0.5);
	//shader.setFloat("ks", 0.5);
	

	//Definindo a fonte de luz pontual
	shader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
	shader.setVec3("lightColor", lightColor.r, lightColor.g, lightColor.b);


	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader.ID, "colorBuffer"), 0);



	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);


		float angle = (GLfloat)glfwGetTime();

		glm::mat4 model = glm::mat4(1);

		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));

		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

		}

		//Atualizando a posição e orientação da câmera
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		shader.setMat4("view", glm::value_ptr(view));
		
		//Atualizando o shader com a posição da câmera
		shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

		projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);
		shader.setMat4("projection", glm::value_ptr(projection));

		// Chamada de desenho - drawcall

		for (int i = 0; i < objects.size(); i++) {
			shader.setVec3("lightColor", lightColor.r, lightColor.g, lightColor.b);
			objects[i].update();
			objects[i].draw();
		}
		

		shader.setMat4("model", glm::value_ptr(model));

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W)
	{
		cameraPos += cameraFront * float(0.1);
	}

	if (key == GLFW_KEY_S)
	{
		cameraPos -= cameraFront * float(0.1);
	}

	if (key == GLFW_KEY_A)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * float(0.1);
	}

	if (key == GLFW_KEY_D)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * float(0.1);
	}


	//Ecolha de desenho
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		if (selected < objects.size() && selected > -1) {
			objects[selected].selected = false;
		}
		selected -= 1;
		if (selected < 0) {
			selected = objects.size() - 1;
		}
		if (selected < objects.size() && selected > -1) {
			objects[selected].selected = true;
		}
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		if (selected < objects.size() && selected > -1) {
			objects[selected].selected = false;
		}
		selected += 1;
		if (selected > objects.size()) {
			selected = 0;
		}
		if (selected < objects.size() && selected > -1) {
			objects[selected].selected = true;
		}
		
		

	}

	//translate
	if (key == GLFW_KEY_UP)
	{
		objects[selected].position.z -= 0.1;
	}

	if (key == GLFW_KEY_DOWN)
	{
		objects[selected].position.z += 0.1;
	}

	if (key == GLFW_KEY_LEFT)
	{
		objects[selected].position.x -= 0.1;
	}

	if (key == GLFW_KEY_RIGHT)
	{
		objects[selected].position.x += 0.1;
	}

	if (key == GLFW_KEY_RIGHT_SHIFT)
	{
		objects[selected].position.y += 0.1;
	}

	if (key == GLFW_KEY_RIGHT_CONTROL)
	{
		objects[selected].position.y -= 0.1;
	}

	//scale
	if (key == GLFW_KEY_O)
	{
		objects[selected].scale.x -= 0.1;
		objects[selected].scale.y -= 0.1;
		objects[selected].scale.z -= 0.1;
	}
	if (key == GLFW_KEY_P)
	{
		objects[selected].scale.x += 0.1;
		objects[selected].scale.y += 0.1;
		objects[selected].scale.z += 0.1;
	}

	//rotation



	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		if (axisX == 0.0) {
			axisX = 1.0;
		}
		else {
			axisX = 0.0;
		}
	}
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		if (axisY == 0.0) {
			axisY = 1.0;
		}
		else {
			axisY = 0.0;
		}
	}
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		if (axisZ == 0.0) {
			axisZ = 1.0;
		}
		else {
			axisZ = 0.0;
		}
	}

	if (key == GLFW_KEY_K)
	{
		objects[selected].axis = glm::vec3(axisX, axisY, axisZ);
		objects[selected].angle += 0.5;
	}
	if (key == GLFW_KEY_L)
	{

		objects[selected].axis = glm::vec3(axisX, axisY, axisZ);
		objects[selected].angle -= 0.5;
	}

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float sensitivity = 0.05;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float offsetx = xpos - lastX;
	float offsety = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	offsetx *= sensitivity;
	offsety *= sensitivity;

	pitch += offsety;
	yaw += offsetx;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= yoffset;
}
