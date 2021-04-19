//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

static const GLsizei WIDTH = 1080, HEIGHT = 720; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse         = true;  // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;
static int g_shaderProgram = 0;

#define PI 3.14159265   

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(float3(0.0f, 0.0f, 5.0f));

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE: //на Esc выходим из программы
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
		if (action == GLFW_PRESS)
		{
			if (filling == 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				filling = 1;
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				filling = 0;
			}
		}
		break;
    case GLFW_KEY_1:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case GLFW_KEY_2:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
	default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        g_captureMouse = !g_captureMouse;


    if (g_captureMouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_capturedMouseJustNow = true;
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }

    GLfloat xoffset = float(xpos) - lastX;
    GLfloat yoffset = lastY - float(ypos);  

    lastX = float(xpos);
    lastY = float(ypos);

    if (g_captureMouse)
        camera.ProcessMouseMove(xoffset, yoffset);
}

void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera &camera, GLfloat deltaTime)
{
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}



GLsizei CreateSphere(float radius, int numberSlices, GLuint &vao)
{
    int i, j;

    int numberParallels = numberSlices;
    int numberVertices = (numberParallels + 1) * (numberSlices + 1);
    int numberIndices = numberParallels * numberSlices * 3;

    float angleStep = (2.0f * 3.14159265358979323846f) / ((float) numberSlices);

    std::vector<float> pos(numberVertices * 4, 0.0f);
    std::vector<float> norm(numberVertices * 4, 0.0f);
    std::vector<float> texcoords(numberVertices * 2, 0.0f);

    std::vector<int> indices(numberIndices, -1);

    for (i = 0; i < numberParallels + 1; i++)
    {
        for (j = 0; j < numberSlices + 1; j++)
        {
            int vertexIndex = (i * (numberSlices + 1) + j) * 4;
            int normalIndex = (i * (numberSlices + 1) + j) * 4;
            int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

            pos.at(vertexIndex + 0) = radius * sinf(angleStep * (float) i) * sinf(angleStep * (float) j);
            pos.at(vertexIndex + 1) = radius * cosf(angleStep * (float) i);
            pos.at(vertexIndex + 2) = radius * sinf(angleStep * (float) i) * cosf(angleStep * (float) j);
            pos.at(vertexIndex + 3) = 1.0f;

            norm.at(normalIndex + 0) = pos.at(vertexIndex + 0) / radius;
            norm.at(normalIndex + 1) = pos.at(vertexIndex + 1) / radius;
            norm.at(normalIndex + 2) = pos.at(vertexIndex + 2) / radius;
            norm.at(normalIndex + 3) = 1.0f;

            texcoords.at(texCoordsIndex + 0) = (float) j / (float) numberSlices;
            texcoords.at(texCoordsIndex + 1) = (1.0f - (float) i) / (float) (numberParallels - 1);
        }
    }

    int *indexBuf = &indices[0];

    for (i = 0; i < numberParallels; i++)
    {
        for (j = 0; j < numberSlices; j++)
        {
            *indexBuf++ = i * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

            *indexBuf++ = i * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
            *indexBuf++ = i * (numberSlices + 1) + (j + 1);

            int diff = int(indexBuf - &indices[0]);
            if (diff >= numberIndices)
                break;
        }
    int diff = int(indexBuf - &indices[0]);
    if (diff >= numberIndices)
        break;
    }

    GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), &pos[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), &norm[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vboTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateCone(GLuint& vao, float4 a, float radius, float height, int numberSlices)
{
    float angle = 360 / numberSlices; //в градусах

    std::vector<float> positions =
    {
        a.x, a.y, a.z, a.w, //центр
        a.x,a.y, a.z + height, a.w, //вершина
    };

    std::vector<float> normals =
    {
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
    };

    std::vector<uint32_t> indices;

    for (int i = 0; i < numberSlices; i++)
    {
        positions.push_back(a.x + radius * cos(angle * i * PI / 180));  // x
        positions.push_back(a.y + radius * sin(angle * i * PI / 180)); // y
        positions.push_back(a.z);  // z
        positions.push_back(1.0f);

        normals.push_back(positions.at(i + 2 + 0) / numberSlices);
        normals.push_back(positions.at(i + 2 + 1) / numberSlices);
        normals.push_back(positions.at(i + 2 + 2) / numberSlices);
        normals.push_back(1.0f);

        // основание
        indices.push_back(0);
        indices.push_back(i + 2);
        if ((i + 1) == numberSlices) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 2 + 1);
        }

        // шапка
        indices.push_back(1);
        indices.push_back(i + 2);
        if ((i + 1) == numberSlices) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 2 + 1);
        }
    }

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateCylinder(GLuint& vao, float4 a, float radius, float height, int numberSlices)
{
    float angle = 360 / numberSlices; //в градусах

    std::vector<float> positions =
    {
        a.x, a.y, a.z, a.w, //центр снизу
        a.x,a.y, a.z + height, a.w, // центр сверху
    };

    std::vector<float> normals =
    {
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
    };

    //нахождение точек
    for (int i = 0; i < numberSlices; i++)
    {
        //нижние точки
        positions.push_back(a.x + radius * cos(angle * i * PI / 180));  // x
        positions.push_back(a.y + radius * sin(angle * i * PI / 180)); // y
        positions.push_back(a.z);  // z
        positions.push_back(1.0f);

        //верхние точки
        positions.push_back(a.x + radius * cos(angle * i * PI / 180));  // x
        positions.push_back(a.y + radius * sin(angle * i * PI / 180)); // y
        positions.push_back(a.z + height);  // z
        positions.push_back(1.0f);

        normals.push_back(positions.at(i + 2 + 0) / numberSlices);
        normals.push_back(positions.at(i + 2 + 1) / numberSlices);
        normals.push_back(positions.at(i + 2 + 2) / numberSlices);
        normals.push_back(1.0f);

        normals.push_back(positions.at(i + 2 + 0) / numberSlices);
        normals.push_back(positions.at(i + 2 + 1) / numberSlices);
        normals.push_back(positions.at(i + 2 + 2) / numberSlices);
        normals.push_back(1.0f);
    }

    std::vector<uint32_t> indices;

    //добавление индексов
    for (int i = 0; i < 2 * numberSlices; i += 2)
    {
        // нижнее основание
        indices.push_back(0);
        indices.push_back(i + 2);
        if ((i + 2) == 2 * numberSlices) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 2 + 2);
        }
        // верхнее основание
        indices.push_back(1);
        indices.push_back(i + 2 + 1);
        if ((i + 2) == 2 * numberSlices) {
            indices.push_back(3);
        }
        else {
            indices.push_back(i + 2 + 3);
        }

        //грани
        //первая половинка
        indices.push_back(i + 2);
        indices.push_back(i + 2 + 1);
        if ((i + 2) == 2 * numberSlices) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 2 + 2);
        }
        //вторая половинка
        indices.push_back(i + 2 + 1);
        if ((i + 2) == 2 * numberSlices) {
            indices.push_back(2);
            indices.push_back(3);
        }
        else {
            indices.push_back(i + 2 + 2);
            indices.push_back(i + 2 + 3);
        }

    }

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreatePlane(GLuint& vao, float4 a, float size)
{
    std::vector<float> positions =
    {
        a.x, a.y, a.z, a.w,
        a.x + size, a.y, a.z, a.w,
        a.x, a.y + size, a.z, a.w,
        a.x + size, a.y + size, a.z, a.w
    };

    std::vector<float> normals =
    {
        0.0f,0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    };

    std::vector<uint32_t> indices =
    {
        0u, 1u, 2u,
        1u, 2u, 3u
    };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateParallel(GLuint& vao, float4 a, float size)
{

    std::vector<float> positions =
    { a.x, a.y, a.z, a.w,
      a.x, a.y, a.z + size, a.w,
      a.x + size, a.y, a.z + size, a.w,
      a.x + size, a.y, a.z, a.w,
      a.x, a.y + size, a.z, a.w,
      a.x, a.y + size, a.z + size, a.w,
      a.x + size, a.y + size, a.z + size, a.w,
      a.x + size, a.y + size, a.z, a.w,
    };

    std::vector<float> normals =
    {
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
    };

    std::vector<uint32_t> indices =
    {
        0u, 1u, 2u,
        0u, 3u, 2u,
        3u, 2u, 6u,
        3u, 7u, 6u,
        0u, 3u, 7u,
        0u, 4u, 7u,
        1u, 2u, 6u,
        1u, 5u, 6u,
        4u, 7u, 6u,
        4u, 5u, 6u,
        0u, 1u, 5u,
        0u, 4u, 5u,
    };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

int initGL()
{
	int res = 0;

	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	//выводим в консоль некоторую информацию о драйвере и контексте opengl
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << "Controls: "<< std::endl;
    std::cout << "press right mouse button to capture/release mouse cursor  "<< std::endl;
    std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
    std::cout << "press ESC to exit" << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	if(!glfwInit())
        return -1;
	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 
    GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 
	//регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
	glfwSetKeyCallback        (window, OnKeyboardPressed);  
	glfwSetCursorPosCallback  (window, OnMouseMove); 
    glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback     (window, OnMouseScroll);
	glfwSetInputMode          (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
	
    if(initGL() != 0) 
		return -1;
	
    //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	//создание шейдерной программы из двух файлов с исходниками шейдеров
	//используется класс-обертка ShaderProgram
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "../shaders/vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "../shaders/lambert.frag";
	ShaderProgram lambert(shaders); GL_CHECK_ERRORS;
    
    //Создаем и загружаем геометрию поверхности
    GLuint vaoSphere;
    float radius = 1.0f;
    GLsizei sphereIndices = CreateSphere(radius, 64, vaoSphere);
    GLuint vaoCone;
    GLsizei coneIndices = CreateCone(vaoCone, float4(0.0f, 3.0f, 0.0f, 1.0f), 1.0f, 2.0f, 50);
    GLuint vaoCylinder;
    GLsizei cylinderIndices = CreateCylinder(vaoCylinder, float4(2.0f, -3.0f, 2.0f, 1.0f), 0.5f, 1.0f, 20);
    GLuint vaoPlane;
    GLsizei planeIndices = CreatePlane(vaoPlane, float4(-4.0f, 0.0f, 5.0f, 1.0f), 2.0f);
    GLuint vaoParallel;
    GLsizei parallelIndices = CreateParallel(vaoParallel, float4(2.0f, 5.0f, 1.0f, 1.0f), 2.0f);
    GLuint vaoNeptune;
    GLsizei NeptuneIndices = CreateSphere(0.33f, 20, vaoNeptune);
    
    glViewport(0, 0, WIDTH, HEIGHT);  GL_CHECK_ERRORS;
    glEnable(GL_DEPTH_TEST);  GL_CHECK_ERRORS;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float step = 0.0f;
	//цикл обработки сообщений и отрисовки сцены каждый кадр
	while (!glfwWindowShouldClose(window))
	{
		//считаем сколько времени прошло за кадр
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
        glfwPollEvents();

        doCameraMovement(camera, deltaTime);

		//очищаем экран каждый кадр
		glClearColor(0.9f, 0.95f, 0.97f, 1.0f); GL_CHECK_ERRORS;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

        lambert.StartUseShader(); GL_CHECK_ERRORS;

        float4x4 view       = camera.GetViewMatrix();
        float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);
	    

        lambert.SetUniform("view", view);       GL_CHECK_ERRORS;
        lambert.SetUniform("projection", projection); GL_CHECK_ERRORS;

        glBindVertexArray(vaoSphere);
        {
            float4x4 model1;
            model1 = transpose(mul(translate4x4(float3(0.0f, 0.0f, 0.0f)), scale4x4(float3(1.0f, 1.0f, 1.0f))));
            lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        }
        glBindVertexArray(0); GL_CHECK_ERRORS;
        glBindVertexArray(vaoCone); GL_CHECK_ERRORS;
        {
            float4x4 model;
            model = transpose(translate4x4(float3(-1.0f, 0.0f, 0.0f)));
            lambert.SetUniform("model", model); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, coneIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        }
        glBindVertexArray(0); GL_CHECK_ERRORS;
        glBindVertexArray(vaoCylinder); GL_CHECK_ERRORS;
        {
            float4x4 model;
            model = transpose(translate4x4(float3(-1.0f, 0.0f, 0.0f)));
            lambert.SetUniform("model", model); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, cylinderIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        }
        glBindVertexArray(0); GL_CHECK_ERRORS;
        glBindVertexArray(vaoPlane); GL_CHECK_ERRORS;
        {
            float4x4 model;
            model = transpose(translate4x4(float3(-1.0f, 0.0f, 0.0f)));
            lambert.SetUniform("model", model); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, planeIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        }
        glBindVertexArray(0); GL_CHECK_ERRORS;
        glBindVertexArray(vaoParallel); GL_CHECK_ERRORS;
        {
            float4x4 model;
            model = transpose(translate4x4(float3(2.5f, 0.0f, -10.0f)));
            lambert.SetUniform("model", model); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, parallelIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
          
            for (int i = 1; i < 20; i++) {
                model = transpose(mul(rotate_Y_4x4((i * 20 * 1.0f) * LiteMath::DEG_TO_RAD), translate4x4(float3(1.0f, (i * - 0.4f),-10.0f)))); // пропорции
                lambert.SetUniform("model", model); GL_CHECK_ERRORS;
                glDrawElements(GL_TRIANGLE_STRIP, parallelIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
            }
        
        }
        glBindVertexArray(vaoNeptune);
        {
            float4x4 model;
            model = transpose(mul(rotate_Y_4x4((step * 1.0f) * LiteMath::DEG_TO_RAD), translate4x4(float3(9.0f, 0.0f, -20.0f))));
            lambert.SetUniform("model", model); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, NeptuneIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;

            float4x4 model1;
            model1 = transpose(mul(rotate_Y_4x4((-step * 4.5f) * LiteMath::DEG_TO_RAD), translate4x4(float3(9.0f, 5.0f, -20.0f))));
            lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
            glDrawElements(GL_TRIANGLE_STRIP, NeptuneIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
        }

        glBindVertexArray(0); GL_CHECK_ERRORS;
        lambert.StopUseShader(); GL_CHECK_ERRORS;
        glfwSwapBuffers(window);

        step += 0.05;
	}
    glDeleteVertexArrays(1, &vaoSphere);
    glDeleteVertexArrays(1, &vaoNeptune);
	
    glfwTerminate();
	return 0;


}
