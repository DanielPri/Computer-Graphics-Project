/*
MIT License

Copyright (c) 2017   Chris McArthur, prince.chrismc(at)gmail(dot)com
                     Daniel P, privorotskyd(at)gmail(dot)com
                     Nicholas G, dj_nick_gattuso(at)hotmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "camera.h"
#include "GlfwWindow.h"
#include "Shader.h"
#include "ObjLoader.h"
#include "World.h"
#include "irrKlang/irrKlang.h"
#include "Light.h"

#include "GL/glew.h"                            // include GL Extension Wrangler

#include <string>
#include <iostream>
#include <vector>

#pragma comment(lib, "lib/irrKlang.lib") // link with irrKlang.dll
using namespace irrklang;

// Function Declarations
bool SetupGlew();
bool SetupShaders();
int ExitOnEnter();
void SetCalbacks(std::shared_ptr<GlfwWindow> window);
void ClearFrame();
void PerFrameCalc();

// Gloabal objects
Camera g_camera(glm::vec3(64.0f, 3.0f, 64.0f));
std::shared_ptr<World> g_World;

ISoundEngine* engine;

int main()
{
   std::cout << "Welcome to Never Green Grove!" << std::endl;

   auto window = GlfwWindowFactory::GetInstance()->CreateNewWindow("Never Green Grove - Prepare to get lost!");
   if (!window->IsValid()) return ExitOnEnter();
   SetCalbacks(window);

   if(!SetupGlew()) return ExitOnEnter();

   if (!SetupShaders()) return ExitOnEnter();
   auto shaderProgram = ShaderLinker::GetInstance();

  engine = createIrrKlangDevice();

   if (!engine)
   {
      printf("Could not startup engine\n");
      return 0; // error starting up the engine
   }

   ISound* snd = engine->play2D("assets/apocalypse.mp3", true, true); //reference https://www.youtube.com/watch?v=OMoTcNxpUVg
   if (snd) {
	   snd->setPlayPosition(500);
	   snd->setIsPaused(false);
	   snd->drop();
   }

   glEnable(GL_DEPTH_TEST);

   //auto chunk1 = std::make_shared<Chunk>();
   //auto chunk2 = std::make_shared<Chunk>();
   //chunk1->Translate(glm::vec3(-128, 0, -128));
   //chunk1->Translate(glm::vec3(Chunk::ONE_TRANS_UNIT, 0, Chunk::ONE_TRANS_UNIT));
   g_World = std::make_shared<World>();
   // Terrain
   //Chunk terrain1;
   //Chunk terrain2;
   //Chunk terrain3;
   //Chunk terrain4;
   //terrain2.Translate(glm::vec3(Chunk::ONE_TRANS_UNIT, 0, 0));
   //terrain3.Translate(glm::vec3(0, 0, Chunk::ONE_TRANS_UNIT));
   //terrain4.Translate(glm::vec3(Chunk::ONE_TRANS_UNIT, 0, Chunk::ONE_TRANS_UNIT));
   size_t counter = 0;

   Light light;

   // Game loop
   while (!window->ShouldClose())
   {
      PerFrameCalc();                           // Per frame time drift calc - MUST be called before triggering callbacks
      GlfwWindow::TriggerCallbacks();           // For all windows check callbacks
      ClearFrame();                             // Reset background color and z buffer test

      shaderProgram->SetUniformMat4("view_matrix", g_camera.GetViewMatrix());
      shaderProgram->SetUniformMat4("projection_matrix", window->GetProjectionMatrix());

      if (counter % 1500 == 0) engine->play2D("assets/zombie.mp3"); //referenced https://www.youtube.com/watch?v=9ounTccHNsw
	  else if (counter % 750 == 0) { 
		  ISound* snd = engine->play2D("assets/dramaticSound.mp3", false, true); 
		  snd->setPlayPosition(100);
		  snd->setIsPaused(false);
		  snd->drop();
	  }

	  light.Draw(counter++);

      // Render -------------------------------------------------------------------------------------------------------------------------------------
      //terrain1.Draw();
      //terrain2.Draw();
      //terrain3.Draw();
      //terrain4.Draw();
      g_World->Draw();
      g_World->Update(g_camera.GetPos());
      //chunk1->Draw();
      //chunk2->Draw();
      /// Render

      window->NextBuffer(); // swap buffers

      counter++;
   }

   engine->drop(); // delete engine
   return 0;
}

// ------------------------------------------------------------------------------------------------ //
//                                       UTILITY FUNCTIONS                                          //
// ------------------------------------------------------------------------------------------------ //
bool SetupShaders()
{
   VertexShader vertexShader("shaders/vertex.shader");
   FragmentShader fragmentShader("shaders/fragment.shader");
   // make sure they are ready to use
   if (!vertexShader() || !fragmentShader()) return false;

   auto shaderProgram = ShaderLinker::GetInstance();
   if (!shaderProgram->Link(&vertexShader, &fragmentShader)) return false;

   return true;
}

// Times needs to track camera calcs --------------------------------------------------------------
float g_deltaTime = 0.0f;                         // time between current frame and last frame
float g_lastFrame = 0.0f;                         // time of last frame

void PerFrameCalc()
{
   float currentFrame = (float)glfwGetTime();
   g_deltaTime = currentFrame - g_lastFrame;
   g_lastFrame = currentFrame;

   g_camera.moveCamera(g_deltaTime, /*g_World->GetHeight(g_camera.GetPos()) +*/ 3.0f);
}

// Callback declarations for camera ---------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void SetCalbacks(std::shared_ptr<GlfwWindow> window)
{
   // Set the required callback functions
   window->SetFramebufferSizeCallback(framebuffer_size_callback);
   window->SetCursorPosCallback(mouse_callback);
   window->SetScrollCallback(scroll_callback);
   window->SetKeyCallback(key_callback);
   window->CaptureCursor();                     // tell GLFW to capture our mouse
}

int ExitOnEnter()
{
   std::cout << "Press 'enter' to exit." << std::endl;
   std::getline(std::cin, std::string());
   return -1;
}

void ClearFrame()
{
   glClearColor(0.05f, 0.075f, 0.075f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);             // Clear the depth buffer
}

bool SetupGlew()
{
   // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
   glewExperimental = GL_TRUE;
   // Initialize GLEW to setup the OpenGL Function pointers
   if (glewInit() != GLEW_OK)
   {
      std::cout << "Failed to initialize GLEW" << std::endl;
      return false;
   }
   return true;
}

// ------------------------------------------------------------------------------------------------ //
//                                      CALLBACK FUNCTIONS                                        - //
// ------------------------------------------------------------------------------------------------ //
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

   if (key == GLFW_KEY_W && action == GLFW_PRESS)
      g_camera.goForward = true;
   if (key == GLFW_KEY_W && action == GLFW_RELEASE)
      g_camera.goForward = false;

   if (key == GLFW_KEY_S && action == GLFW_PRESS)
      g_camera.goBackward = true;
   if (key == GLFW_KEY_S && action == GLFW_RELEASE)
      g_camera.goBackward = false;

   if (key == GLFW_KEY_A && action == GLFW_PRESS)
      g_camera.goLeft = true;
   if (key == GLFW_KEY_A && action == GLFW_RELEASE)
      g_camera.goLeft = false;

   if (key == GLFW_KEY_D && action == GLFW_PRESS)
      g_camera.goRight = true;
   if (key == GLFW_KEY_D && action == GLFW_RELEASE)
      g_camera.goRight = false;

   if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
      g_camera.goUp = true;
   if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
      g_camera.goUp = false;

   
   if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
	   if (g_camera.godMode) {
		   g_camera.goDown = true;
	   }
	   else {
		   g_camera.sprint = true;
	   }
   }
   if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
	   if (g_camera.godMode) {
		   g_camera.goDown = false;
	   }
	   else {
		   g_camera.sprint = false;
	   }
   }
	   if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		   if (g_camera.godMode) {
			   g_camera.godMode = false; 
			   ISound* snd = engine->play2D("assets/powerDown.mp3", false,true);
			   snd->setPlayPosition(100);
			   snd->setIsPaused(false);
			   snd->drop();
		   }
		   else { g_camera.godMode = true; engine->play2D("assets/powerUp2.mp3", false); }
	   }
   

   if (key == GLFW_KEY_T && action == GLFW_PRESS)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   if (key == GLFW_KEY_L && action == GLFW_PRESS)
     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
   // make sure the viewport matches the new window dimensions; note that width and
   // height will be significantly larger than specified on retina displays.
   glViewport(0, 0, width, height);
}

// Capture mouse mouvements for camera ------------------------------------------------------------
double g_lastX = GlfwWindow::DEFAULT_WIDTH / 2.0;
double g_lastY = GlfwWindow::DEFAULT_HEIGHT / 2.0;
bool g_firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
   // using method from https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.4.camera_class/camera_class.cpp
   if (g_firstMouse)
   {
      g_lastX = xpos;
      g_lastY = ypos;
      g_firstMouse = false;
   }

   float xoffset = float(xpos - g_lastX);
   float yoffset = float(g_lastY - ypos);       // reversed since y-coordinates go from bottom to top

   g_lastX = xpos;
   g_lastY = ypos;

   g_camera.ProcessMouseMovement(xoffset, yoffset);
}

// only used for FOV
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   g_camera.ProcessMouseScroll((float)yoffset);
}
