/* Hello Triangle - c�digo adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gr�fico/Computa��o Gr�fica - Unisinos
 * Vers�o inicial: 7/4/2017
 * �ltima atualiza��o em 01/03/2023
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


// Prot�tipo da fun��o de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
// Prot�tipo da fun��o de callback do mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


// Prot�tipos das fun��es
int setupGeometry();
int loadSimpleOBJ(string filepath, int &nVerts, glm::vec3 color = glm::vec3(1.0,0.0,1.0));
GLuint generateTexture(string filepath);
vector <Object> readConfigurationFile(string path, Shader *shader);

// Dimens�es da janela (pode ser alterado em tempo de execu��o)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX=false, rotateY=false, rotateZ=false;

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

bool firstMouse = true;
float lastX, lastY;
float sensitivity = 0.05;
float pitch = 0.0, yaw = -90.0;

// Fun��o MAIN
int main()
{	
	// Inicializa��o da GLFW
	glfwInit();

	//Muita aten��o aqui: alguns ambientes n�o aceitam essas configura��es
	//Voc� deve adaptar para a vers�o do OpenGL suportada por sua placa
	//Sugest�o: comente essas linhas de c�digo para desobrir a vers�o e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Cria��o da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da fun��o de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	

	glfwSetCursorPos(window,WIDTH / 2, HEIGHT / 2);


	//Desabilita o desenho do cursor 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// GLAD: carrega todos os ponteiros d fun��es da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informa��es de vers�o
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimens�es da viewport com as mesmas dimens�es da janela da aplica��o
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	//GLuint shader.ID = setupShader();
	Shader shader("Phong.vs","Phong.fs");

	glUseProgram(shader.ID);

	//Matriz de view -- posi��o e orienta��o da c�mera
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("view",value_ptr(view));

	//Matriz de proje��o perspectiva - definindo o volume de visualiza��o (frustum)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));

	glEnable(GL_DEPTH_TEST);

	//vector <Object> objects = readConfigurationFile("../../config.json", &shader);
	ifstream in("../../config.json");
	string contents((std::istreambuf_iterator<char>(in)),
		istreambuf_iterator<char>());

	rapidjson::Document document;
	document.Parse(contents.c_str());


	vector <Object> objects;

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
		projection = glm::perspective(glm::radians(document["camera"]["fov"].GetFloat()), (float)width / (float)height, 0.1f, 100.0f);
		shader.setMat4("projection", glm::value_ptr(projection));

		//Modelos
		//Coletar diret�rio
		string folder = document["objects"][i]["folder"].GetString();

		//Coletar modelo
		string objfile = document["objects"][i]["model"].GetString();

		//Coletar mtl
		string mtl = document["objects"][i]["mtl"].GetString();

		//Coletar posi��o
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



	// Loop da aplica��o - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as fun��es de callback correspondentes
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

		//Atualizando a posi��o e orienta��o da c�mera
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		shader.setMat4("view", glm::value_ptr(view));
		
		//Atualizando o shader com a posi��o da c�mera
		shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

		// Chamada de desenho - drawcall

		for (int i = 0; i < objects.size(); i++) {
			objects[i].update();
			objects[i].draw();
		}
		

		shader.setMat4("model", glm::value_ptr(model));

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	
	// Finaliza a execu��o da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Fun��o de callback de teclado - s� pode ter uma inst�ncia (deve ser est�tica se
// estiver dentro de uma classe) - � chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	float cameraSpeed = 0.05;

	if (key == GLFW_KEY_W)
	{
		cameraPos += cameraFront * cameraSpeed;
	}
	if (key == GLFW_KEY_S)
	{
		cameraPos -= cameraFront * cameraSpeed;
	}
	if (key == GLFW_KEY_A)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (key == GLFW_KEY_D)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}



}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//cout << xpos << " " << ypos << endl;
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


// Esta fun��o est� bastante harcoded - objetivo � criar os buffers que armazenam a 
// geometria de um tri�ngulo
// Apenas atributo coordenada nos v�rtices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A fun��o retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do tri�ngulo e as armazenamos de forma
	// sequencial, j� visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do v�rtice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO �nico ou em VBOs separados
	GLfloat vertices[] = {

		//Base da pir�mide: 2 tri�ngulos
		//x    y    z    r    g    b
		-0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
		-0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		 -0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
		  0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
		  0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		 //
		 -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
		  0.0,  0.5,  0.0, 1.0, 1.0, 0.0,
		  0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

		  -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
		  0.0,  0.5,  0.0, 1.0, 0.0, 1.0,
		  -0.5, -0.5, 0.5, 1.0, 0.0, 1.0,

		   -0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
		  0.0,  0.5,  0.0, 1.0, 1.0, 0.0,
		  0.5, -0.5, 0.5, 1.0, 1.0, 0.0,

		   0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
		  0.0,  0.5,  0.0, 0.0, 1.0, 1.0,
		  0.5, -0.5, -0.5, 0.0, 1.0, 1.0,

		  //Ch�o
		  //x    y    z    r    g    b
		  -5.0, -0.5, -5.0, 0.5, 0.5, 0.5,
		  -5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
		   5.0, -0.5, -5.0, 0.5, 0.5, 0.5,

		  -5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
		   5.0, -0.5,  5.0, 0.5, 0.5, 0.5,
		   5.0, -0.5, -5.0, 0.5, 0.5, 0.5

	};

	GLuint VBO, VAO;

	//Gera��o do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conex�o (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Gera��o do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de v�rtices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localiza��o no shader * (a localiza��o dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se est� normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posi��o (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);



	// Observe que isso � permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de v�rtice 
	// atualmente vinculado - para que depois possamos desvincular com seguran�a
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (� uma boa pr�tica desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

int loadSimpleOBJ(string filepath, int &nVerts, glm::vec3 color)
{
	vector <Vertex> vertices;
	vector <GLuint> indices;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> normals;
	vector <GLfloat> vbuffer;

	ifstream inputFile;
	inputFile.open(filepath.c_str());
	if (inputFile.is_open())
	{
		char line[100];
		string sline;
		
		
		
		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;
			
			string word;
			
			istringstream ssline(line);
			ssline >> word;

			//cout << word << " ";
			if (word == "v")
			{
				Vertex v;

				ssline >> v.position.x >> v.position.y >> v.position.z;
				v.color.r = color.r;  v.color.g = color.g;  v.color.b = color.b;
				
				vertices.push_back(v);
			}
			if (word == "vt")
			{
				glm::vec2 vt;

				ssline >> vt.s >> vt.t;
				
				texCoords.push_back(vt);
			}
			if (word == "vn")
			{
				glm::vec3 vn;

				ssline >> vn.x >> vn.y >> vn.z;

				normals.push_back(vn);
			}
			if (word == "f")
			{
				string tokens[3];
				
				ssline >> tokens[0] >> tokens[1] >> tokens[2];
				
				for (int i = 0; i < 3; i++)
				{
					//Recuperando os indices de v
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					indices.push_back(index);
					
					vbuffer.push_back(vertices[index].position.x);
					vbuffer.push_back(vertices[index].position.y);
					vbuffer.push_back(vertices[index].position.z);
					vbuffer.push_back(vertices[index].color.r);
					vbuffer.push_back(vertices[index].color.g);
					vbuffer.push_back(vertices[index].color.b);
				
					//Recuperando os indices de vts
					tokens[i] = tokens[i].substr(pos + 1);
					pos = tokens[i].find("/");
					token = tokens[i].substr(0, pos);
					index = atoi(token.c_str()) - 1;

					vbuffer.push_back(texCoords[index].s);
					vbuffer.push_back(texCoords[index].t);

					//Recuperando os indices de vns
					tokens[i] = tokens[i].substr(pos + 1);
					index = atoi(tokens[i].c_str()) - 1;

					vbuffer.push_back(normals[index].x);
					vbuffer.push_back(normals[index].y);
					vbuffer.push_back(normals[index].z);
				}
			}

		}

	}
	else
	{
		cout << "Problema ao encontrar o arquivo " << filepath << endl;
	}
	inputFile.close();

	GLuint VBO, VAO;

	nVerts = vbuffer.size() / 11; //Provis�rio

	//Gera��o do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conex�o (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(GLfloat), vbuffer.data(), GL_STATIC_DRAW);

	//Gera��o do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de v�rtices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localiza��o no shader * (a localiza��o dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se est� normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 

	//Atributo posi��o (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo coordenada de textura (s, t)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Atributo normal do v�rtice (x, y, z)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);


	// Observe que isso � permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de v�rtice 
	// atualmente vinculado - para que depois possamos desvincular com seguran�a
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (� uma boa pr�tica desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;

}

GLuint generateTexture(string filepath)
{
	GLuint texID;

	// Gera o identificador da textura na mem�ria
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
	
	if (data)
	{
		if (nrChannels == 3) //jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
				data);
		}
		else //png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}