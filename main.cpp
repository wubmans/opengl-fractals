#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#include "camera.h"

const unsigned int PARTICLES = 1024 * 64;

const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 1024;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

glm::vec4 positions[ PARTICLES ];
glm::vec4 velocities[ PARTICLES ];

glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
glm::mat4 projection = glm::perspective(70.0f, 4.0f/3.0f, 0.1f, 100.0f );
glm::mat4 view = glm::lookAt( glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0., 0., 0.), glm::vec3(0., 1., 0.) );

GLuint positionBuffer;
GLuint velocitiesBuffer;
GLuint vao;

float dt = 0.0f;
float last_interval = 0.0f;
int frameCount = 0;
float interval = 0.0f;

Camera camera(glm::vec3(0, 0, 3.0f), glm::vec3(0, 0, -1.0f), glm::vec3(0, 1.0f, 0.0));


float camera_yaw = 0.0f;
float camera_pitch = 0.0f;

glm::vec3 camera_position(0.0f, 0.0f, 10.0f);
glm::vec3 camera_position_delta(0.0, 0.0f, 0.0f);
glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
glm::vec3 camera_look_at;
glm::vec3 camera_direction;

float r = 100.0f;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

glm::vec3 last_position(0.0f);
glm::vec3 current_position(0.0f);
glm::vec3 rotation;
float angle;

glm::vec3 toScreenCoord( double x, double y ) {
    int windowWidth = 1920;
    int windowHeight = 1080;

    glm::vec3 coord(0.0f);
    
    coord.x =  (2 * x - windowWidth ) / windowWidth;
    coord.y = -(2 * y - windowHeight) / windowHeight;
    
    /* Clamp it to border of the windows, comment these codes to allow rotation when cursor is not over window */
    coord.x = glm::clamp( coord.x, -1.0f, 1.0f );
    coord.y = glm::clamp( coord.y, -1.0f, 1.0f );
    
    float length_squared = coord.x * coord.x + coord.y * coord.y;
    if( length_squared <= 1.0 )
        coord.z = sqrt( 1.0 - length_squared );
    else
        coord = glm::normalize( coord );

    printf("%f, %f, %f\n\n", coord.x, coord.y, coord.z);
    
    return coord;
}

bool initialized;


static void cursor_position_callback(GLFWwindow* window, double x, double y)
{

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    
    if (state != GLFW_PRESS)
    {
        initialized = false;
        return;
    }

    if (!initialized)
    {
        last_position = toScreenCoord(x, y);
        initialized = true;
        return;
    }

    

    glm::vec3 camera_axis;

     /* Tracking the subsequent */
    current_position = toScreenCoord( x, y );

    
    
    /* Calculate the angle in radians, and clamp it between 0 and 90 degrees */
    angle = acos( std::min(1.0f, glm::dot(current_position, last_position)));
    
    /* Cross product to get the rotation axis, but it's still in camera coordinate */
    camera_axis  = glm::cross( last_position, current_position );

    glm::mat3 c2o = glm::inverse(view) * model;

    // /glm::mat4 m = glm::rotate(glm::degrees(angle) * 0.1f, camera_axis );

    glm::vec3 model_axis = c2o * camera_axis;
    model = glm::scale(glm::rotate(glm::mat4(1.0f),  glm::degrees(angle) * 0.1f, camera_axis ), glm::vec3(0.1f));

    // last_position = current_position;

}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    printf("%f\n", r);
    //printf("%f", yoffset);
    r -= yoffset;
}


GLchar* LoadShader(const std::string &file) {

    std::ifstream shaderFile;
    long shaderFileLength;

    shaderFile.open(file, std::ios::binary);

    if (shaderFile.fail()) 
    {
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

    updateCamera();

    glUseProgram(shaderProgram);
    
    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

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
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    GLenum err = glewInit();
    
    // view = glm::lookAt(cameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    updateCamera();

    for (size_t i = 0; i < PARTICLES; i++)
    {
        positions[i].x = glm::gaussRand(0.0f, 0.5f);
        positions[i].y = glm::gaussRand(0.0f, 0.5f);
        positions[i].z = glm::gaussRand(0.0f, 0.5f);
        positions[i].w = 0.2f;

        velocities[i].x = glm::linearRand(-0.001f, 0.001f);
        velocities[i].y = glm::linearRand(-0.001f, 0.001f);
        velocities[i].z = glm::linearRand(-0.001f, 0.001f);
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

    float aspect = (float) 19200.f / 1080.0f;

    //projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
    projection = glm::perspective(glm::radians(45.0f), (float) 1920 /(float) 1080, 0.1f, 1000.0f);
    view = glm::lookAt( glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0., 0., 0.), glm::vec3(0., 1., 0.) );


    while (!glfwWindowShouldClose(window))
    {
        //glUniform1fv(glGetUniformLocation(computeProgram, "dt"), 1, &dt);
        loop(window, computeProgram, shaderProgram);

        frameCount++;
        interval += glfwGetTime();
        dt = (interval - last_interval) * 10.0f;
        
        last_interval = interval;
        glfwSetTime(0.0f);

        if (frameCount % 100 == 0)
        {
            //printf("fps: %f\n", frameCount / interval);
        }

        glfwPollEvents();
    }

}

