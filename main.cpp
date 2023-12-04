#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

const unsigned int PARTICLES = 1024 * 1024;

const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 1024;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

glm::vec4 positions[ PARTICLES ];
glm::vec4 velocities[ PARTICLES ];

glm::mat4 projection;
glm::mat4 view;

GLuint positionBuffer;
GLuint velocitiesBuffer;
GLuint vao;

float dt = 0.0f;
float last_interval = 0.0f;
int frameCount = 0;
float interval = 0.0f;


float camera_yaw;
float camera_pitch;

glm::vec3 cameraPosition(0.0f, 0.0f, -50.0f);

glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_right = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 1.0f);


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_W)
    {
        cameraPosition += camera_forward;
        std::cout << "fowrad!";
    }

    if (key == GLFW_KEY_S)
    {
         cameraPosition -= camera_forward;
    }

    if (key == GLFW_KEY_A)
    {
        cameraPosition -= camera_right;
    }

    if (key == GLFW_KEY_D)
    {
        cameraPosition += camera_right;
    }
}

double last_xpos = -1;
double last_ypos = -1;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
        if (last_xpos == -1  || last_ypos == -1)
        {
            last_xpos = xpos;
            last_ypos = ypos;
            return;
        }
 
        camera_yaw   -= (xpos - last_xpos) * 0.1f;
        camera_pitch -= (ypos - last_ypos) * 0.1f;


        last_xpos = xpos;
        last_ypos = ypos;
        // make sure that when pitch is out of bounds, screen doesn't get flipped

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


void updateCamera()
{

    glm::vec3 temp_front;
    temp_front.x = cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch));
    temp_front.y = sin(glm::radians(camera_pitch));
    temp_front.z = sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch));
    
    camera_forward = glm::normalize(temp_front);
    camera_right = glm::normalize(glm::cross(camera_forward, glm::vec3(0.0f, 1.0f, 0.0f)));  
    camera_up    = glm::normalize(glm::cross(camera_right, camera_forward));

    view  = glm::lookAt(cameraPosition, cameraPosition + camera_forward, camera_up);
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

    // view = glm::lookAt(glm::vec3(0.0f, 0.0f, z), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // create transformations
    glm::mat4 model         = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    // glm::mat4 view          = glm::mat4(1.0f);
    glm::mat4 projection    = glm::mat4(1.0f);

    updateCamera();
  
    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
    
    projection = glm::perspective(glm::radians(45.0f), (float) 1920 / (float) 1080, 0.1f, 100.0f);

    glUseProgram(shaderProgram);
    
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);



    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, PARTICLES);

    glfwSwapBuffers(window);
    glfwSwapInterval(0);
}



int main()
{
    srand(time(NULL));

    GLFWwindow* window;

	if (!glfwInit())
    {
		return -1;
    }
    

	window = glfwCreateWindow(1920, 1080, "Hello World", NULL  /*glfwGetPrimaryMonitor() */, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    GLenum err = glewInit();
    
    view = glm::lookAt(cameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // );

    for (size_t i = 0; i < PARTICLES; i++)
    {
        positions[i].x = glm::linearRand(-1.0f, 1.0f) * 100.0f;
        positions[i].y = glm::linearRand(-1.0f, 1.0f) * 100.0f;
        positions[i].z = glm::linearRand(-1.0f, 1.0f) * 100.0f;
        positions[i].w = 1.0f;

        velocities[i].x = ((rand() % 1000 / 1000) - 0.5f) * 10.f;
        velocities[i].y = ((rand() % 1000 / 1000) - 0.5f) * 10.f;
    }

    // vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // ssbo
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES * sizeof(glm::vec4), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (char*) 0 + 0 * sizeof(GLfloat));

    glGenBuffers(1, &velocitiesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, velocitiesBuffer);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES * sizeof(glm::vec4), velocities, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesBuffer);

    GLuint shaderProgram = createShaderProgram();
    GLuint computeProgram = createComputeProgram();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    glDisable(GL_CULL_FACE);

    glPointSize(2.0);     
    glViewport(0, 0, 1920, 1080);

    projection = glm::perspective(glm::radians(90.0f), 1920.0f / 1080.0f, 0.5f, 1000.0f);

    while (!glfwWindowShouldClose(window))
    {
        //glUniform1fv(glGetUniformLocation(computeProgram, "dt"), 1, &dt);
        loop(window, computeProgram, shaderProgram);

        frameCount++;
        interval += glfwGetTime();
        dt = (interval - last_interval) / 10.0f;
        
        last_interval = interval;
        glfwSetTime(0.0f);

        if (frameCount % 100 == 0)
        {
            printf("fps: %f\n", frameCount / interval);
        }

        glfwPollEvents();
    }

}
