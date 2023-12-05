#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "FlyingCamera.h"

const unsigned int PARTICLES = 1024 * 64;

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

FlyingCamera camera(90.0f, 0.0f, 0.0f, 10.0f, 0.1f, 1000.0f);


float camera_yaw = 0.0f;
float camera_pitch = 0.0f;

glm::vec3 camera_position(0.0f, 0.0f, 10.0f);
glm::vec3 camera_position_delta(0.0, 0.0f, 0.0f);
glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
glm::vec3 camera_look_at;
glm::vec3 camera_direction;

// glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);
// glm::vec3 camera_right;
// glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 1.0f);


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_W)
    {
        camera.moveForward(0.1f);
    }

    if (key == GLFW_KEY_S)
    {
        camera.moveBackward(0.1f);
    }

    if (key == GLFW_KEY_A)
    {
        camera.moveLeft(0.1f);
    }

    if (key == GLFW_KEY_D)
    {
        camera.moveRight(0.1f);
    }
    
    if (key == GLFW_KEY_Q)
    {
        camera.setYaw(-5.0f);
    }

    if (key == GLFW_KEY_E)
    {
        camera.setYaw(5.0f);
    }
    
    setbuf(stdout, NULL);
   
    printf("%.2f, %.2f, %.2f (%.2f, %.2f)\n", camera.loc.x , camera.loc.y , camera.loc.z, camera.yaw, camera.pitch);
}

double last_xpos;
double last_ypos;

bool mouseSet = false;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
 
        if (!mouseSet) {
            last_xpos = xpos;
            last_ypos = ypos;
            mouseSet = true;
            return;
        }

        double dx = xpos - last_xpos;
        double dy = ypos - last_ypos;

        printf("%.2f, %.2f, %.2f, %.2f\n", xpos, ypos, last_xpos, last_ypos);

        printf("%.2f, %.2f\n", dx, dy);

        last_xpos = xpos;
        last_ypos = ypos;

        

        camera.setYaw(dx / 3.0f );
        camera.setPitch(- dy / 3.0f);
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

    // glm::vec3 temp_front;

    // temp_front.x = sin(camera_yaw);
    // temp_front.y = -(sin(camera_pitch)*cos(camera_yaw));
    // temp_front.z = -(cos(camera_pitch)*cos(camera_yaw));
    
    // // printf("%.2f, %.2f, %.2f\n", temp_front.x , temp_front.y , temp_front.z);

    // printf("%.2f, %.2f\n", camera_pitch, camera_yaw);
    // camera_forward = glm::normalize(temp_front);
    // camera_right = glm::normalize(glm::cross(camera_forward, glm::vec3(0.0f, 1.0f, 0.0f)));  
    // camera_up    = glm::normalize(glm::cross(camera_right, camera_forward));

    // camera_direction = glm::normalize(camera_look_at - camera_position);

    // //detmine axis for pitch rotation
    // glm::vec3 axis = glm::cross(camera_direction, camera_up);
    // //compute quaternion for pitch based on the camera pitch angle
    // glm::quat pitch_quat = glm::angleAxis(camera_pitch, axis);
    // //determine heading quaternion from the camera up vector and the heading angle
    // glm::quat yaw_quat = glm::angleAxis(camera_yaw, camera_up);
    // //add the two quaternions
    // glm::quat temp = glm::cross(pitch_quat, yaw_quat);
    // temp = glm::normalize(temp);
    // //update the direction from the quaternion
    // camera_direction = glm::rotate(temp, camera_direction);
    // camera_position += camera_position_delta;

    // // camera_yaw *= .5;
    // // camera_pitch *= .5;
    // camera_position_delta = camera_position_delta * .8f;

    view  = glm::lookAt(camera.loc, camera.loc + camera.dir, glm::vec3(0.0f, 1.0f, 0.0f));
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
  
    projection = glm::perspective(glm::radians(45.0f), (float) 1920 / (float) 1080, 0.1f, 100.0f);

    glUseProgram(shaderProgram);
    
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
    
    // view = glm::lookAt(cameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    updateCamera();

    for (size_t i = 0; i < PARTICLES; i++)
    {
        positions[i].x = glm::linearRand(-1.0f, 1.0f);
        positions[i].y = glm::linearRand(-1.0f, 1.0f);
        positions[i].z = glm::linearRand(-1.0f, 1.0f);
        positions[i].w = 1.0f;

        velocities[i].x = glm::gaussRand(0.0f, 0.6f);
        velocities[i].y = glm::gaussRand(0.0f, 0.6f);
        velocities[i].z = glm::gaussRand(0.0f, 0.6f);
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
        dt = (interval - last_interval) / 100.0f;
        
        last_interval = interval;
        glfwSetTime(0.0f);

        if (frameCount % 100 == 0)
        {
            //printf("fps: %f\n", frameCount / interval);
        }

        glfwPollEvents();
    }

}
