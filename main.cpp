#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

const unsigned int PARTICLES = 1024 * 1024;

const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 1024;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

glm::vec4 positions[ PARTICLES ];

GLuint positionBuffer;
GLuint vao;

float dt;
float delta;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

GLchar* LoadShader(const std::string &file) {

  std::ifstream shaderFile;
  long shaderFileLength;

  shaderFile.open(file, std::ios::binary);

  if (shaderFile.fail()) {
    throw std::runtime_error("COULD NOT FIND SHADER FILE");
  }

  shaderFile.seekg(0, shaderFile.end);
  shaderFileLength = shaderFile.tellg();
  shaderFile.seekg(0, shaderFile.beg);

  GLchar *shaderCode = new GLchar[shaderFileLength + 1];
  shaderFile.read(shaderCode, shaderFileLength);

  shaderFile.close();

  shaderCode[shaderFileLength] = '\0';

  return shaderCode;
}

GLuint createComputeProgram()
{
    GLchar infolog[512];

    const GLchar* computeCode = LoadShader("cursor.glsl");

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeCode, nullptr);
    glCompileShader(computeShader);
    glGetShaderInfoLog(computeShader, 512, nullptr, infolog);
    std::cout << infolog << std::endl;

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    
    glGetProgramInfoLog(computeProgram, 512, nullptr, infolog);
    std::cout << infolog << std::endl;

    glDeleteShader(computeShader);

    return computeProgram;
}

GLuint createShaderProgram()
{
    GLchar infolog[512];

    const GLchar* vertCode = LoadShader("shader.vert");
    const GLchar* fragCode = LoadShader("shader.frag");
    
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertShader, 1, &vertCode, nullptr);
    glShaderSource(fragShader, 1, &fragCode, nullptr);

    glCompileShader(vertShader);
    glGetShaderInfoLog(vertShader, 512, nullptr, infolog);
    std::cout << infolog << std::endl;
    glCompileShader(fragShader);
    glGetShaderInfoLog(fragShader, 512, nullptr, infolog);
    std::cout << infolog << std::endl;

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    glGetProgramInfoLog(shaderProgram, 512, nullptr, infolog);
    std::cout << infolog << std::endl;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return shaderProgram;
}

void loop(GLFWwindow* window, GLuint computeProgram, GLuint shaderProgram)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // compute phase

    glUseProgram(computeProgram);
    glUniform1fv(glGetUniformLocation(computeProgram, "dt"), 1, &dt);
    glDispatchCompute(PARTICLES / 128, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // shader phase
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, PARTICLES);
  

    glfwSwapBuffers(window);
    glfwSwapInterval(0);
}

int frameCount = 0;
float interval;

int main()
{
    srand(time(NULL));

    GLFWwindow* window;

	if (!glfwInit())
		return -1;

    

	window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    GLenum err = glewInit();

    for (size_t i = 0; i < PARTICLES; i++)
    {
        positions[i] = glm::gaussRand(glm::vec4(0, 0, 0, 1), glm::vec4(1.0, 1.0, 0, 0));
    }

    // vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // ssbo
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES * sizeof(glm::vec4), &positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (char*) 0 + 0 * sizeof(GLfloat));


    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);

    GLuint shaderProgram = createShaderProgram();
    GLuint computeProgram = createComputeProgram();

    while (!glfwWindowShouldClose(window))
    {
        loop(window, computeProgram, shaderProgram);
        dt += 0.01f;
        frameCount++;
        interval += glfwGetTime();
        glfwSetTime(0.0f);

        if (frameCount % 100 == 0)
        {
            printf("fps: %f\n", frameCount / interval);
        }

        glfwPollEvents();
    }

}