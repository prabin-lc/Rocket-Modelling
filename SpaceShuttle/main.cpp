#include "main.h"
#include "./src/mesh.h"
#include "./src/model.h"

#define SCALE 0.2f

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float angleX = 0.0f, angleY = -90.0f, angleZ = 0.0f;
float scale = 1.0f;
//glm::vec3 front(camera.Front), rright(camera.Right), up(camera.Up);
bool firstMouse = true;

bool spotEnabled = false;
bool spotKeyPressed = false;

bool animating = false;
float rocketLastYTranslation = 0.f;
float animationRunTime=0.f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void init();
void checkFailure(GLFWwindow *window);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubemap(vector<std::string> faces);



int main()
{

	init();
	GLFWwindow* window= window = glfwCreateWindow(WIDTH, HEIGHT, "Space Shuttle Simulation", NULL, NULL);
	{
		checkFailure(window);

		glViewport(0, 0, WIDTH, HEIGHT);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

	}
	// configure global opengl state
	// -----------------------------
	
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glEnable(GL_CULL_FACE);
	Shader ourShader("./src/shaders/shader.vs","./src/shaders/shader.fs");
	Shader lampShader("./src/shaders/lamp.vs", "./src/shaders/lamp.fs");
	Shader skyboxShader("./src/shaders/skybox.vs", "./src/shaders/skybox.fs");


	// load models
	// -----------
	//Model houseModel("../Resources/house_skp/house.obj");
	//Model houseModel("../Resources/nanosuit/nanosuit.obj");
	//Model houseModel("../Resources/oggy/house.obj");
	//Model houseModel("../Resources/house1/House N111111.3DS");
	//Model houseModel("../Resources/house2/house2.max");
	//Model houseModel("../Resources/house3/source/nivelles 9.fbx");
	//Model houseModel("../Resources/rocket2/rocket.obj");
	Model rocket("../Resources/rocket3/rocket.obj");
	Model base("../Resources/base/base.obj");
	//Model flame("../Resources/flame/flame.obj");
	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.5f,  2.f,  -0.0f),
		glm::vec3(0.5f, 0.4f, -1.0f),
		glm::vec3(0.0f,  10.0f, -24.0f),
		glm::vec3(0.0f,  1.0f, -2.0f)
	};

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};


	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	{
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}

	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//cubemap
	vector<std::string> faces
	{
			"../Resources/skybox1/right.jpg",
			"../Resources/skybox1/left.jpg",
			"../Resources/skybox1/top.jpg",
			"../Resources/skybox1/bottom.jpg",
			"../Resources/skybox1/front.jpg",
			"../Resources/skybox1/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//input
		processInput(window);
		// render
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ourShader.use();
		{
			ourShader.setVec3("viewPos", camera.Position);

			/*
			   Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
			   the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
			   by defining light types as classes and set their values in there, or by using a more efficient uniform approach
			   by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
			*/
			// directional light
			ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
			ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
			ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
			// point light 1
			ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
			ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[0].constant", 1.0f);
			ourShader.setFloat("pointLights[0].linear", 0.09);
			ourShader.setFloat("pointLights[0].quadratic", 0.032);
			// point light 2
			ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
			ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[1].constant", 1.0f);
			ourShader.setFloat("pointLights[1].linear", 0.09);
			ourShader.setFloat("pointLights[1].quadratic", 0.032);
			// point light 3
			ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
			ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[2].constant", 1.0f);
			ourShader.setFloat("pointLights[2].linear", 0.09);
			ourShader.setFloat("pointLights[2].quadratic", 0.032);
			// point light 4
			ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
			ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[3].constant", 1.0f);
			ourShader.setFloat("pointLights[3].linear", 0.09);
			ourShader.setFloat("pointLights[3].quadratic", 0.032);
			// spotLight
			ourShader.setVec3("spotLight.position", camera.Position);
			ourShader.setVec3("spotLight.direction", camera.Front);
			//ourShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
			ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
			ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("spotLight.constant", 1.0f);
			ourShader.setFloat("spotLight.linear", 0.09);
			ourShader.setFloat("spotLight.quadratic", 0.032);
			ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
			ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		}

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::scale(model, scale * glm::vec3(SCALE, SCALE, SCALE));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourShader.setInt("spot", spotEnabled);

		base.Draw(ourShader);

		model = glm::mat4(1.0f);

		if (animating) {
			animationRunTime += deltaTime;
			rocketLastYTranslation += deltaTime * 1.f * glm::exp(animationRunTime - 4);
			model = glm::translate(model, glm::vec3(0.5f, rocketLastYTranslation, -0.4f));
		}
		else {
			model = glm::translate(model, glm::vec3(0.5f, rocketLastYTranslation, -0.40f));
		}
		model = glm::rotate(model, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, -1.0f));
		model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
		// translate it down so it's at the center of the scene
		model = glm::scale(model, scale * glm::vec3(SCALE, SCALE, SCALE));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourShader.setInt("spot", spotEnabled);
		if (rocketLastYTranslation < 2000)
			rocket.Draw(ourShader);

		// also draw the lamp object(s)
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);

		// we now draw as many light bulbs as we have point lights.
		glBindVertexArray(skyboxVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.03f)); // Make it a smaller cube
			lampShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));// remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVAO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}
void checkFailure(GLFWwindow *window)
{
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}


}
void processInput(GLFWwindow *window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		scale -= scale * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		scale += scale * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		angleX += deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		angleX -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		angleY += deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		angleY -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		angleZ -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		angleZ += deltaTime * 45;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spotKeyPressed)
	{
		spotEnabled = !spotEnabled;
		spotKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		spotKeyPressed = false;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		animating = !animating;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		animating = false;
		animationRunTime = 0.f;
		rocketLastYTranslation = 0.f;
	}

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	int state2 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (state == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (firstMouse)
		{
			lastX = (float)xpos;
			lastY = (float)ypos;
			firstMouse = false;
		}

		float xoffset = (float)xpos - lastX;
		float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

		lastX = (float)xpos;
		lastY = (float)ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else
	{
		if (firstMouse == false)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstMouse = true;
		}
	}


}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}