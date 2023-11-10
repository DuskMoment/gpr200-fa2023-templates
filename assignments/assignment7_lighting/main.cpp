#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/texture.h>
#include <ew/procGen.h>
#include <ew/transform.h>
#include <ew/camera.h>
#include <ew/cameraController.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

const int MAX_LIGHTS = 4;

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.1f);

ew::Camera camera;
ew::CameraController cameraController;

struct Light
{
	ew::Vec3 position; //world space
	ew::Vec3 color; //RGB
};

Light lights[MAX_LIGHTS];
int numberOfLights = 1;
int ifPhongInt = 1;
bool  ifPhongBool = true;
bool orbit = false;
struct Material
{
	//all 1-0 ranges
	float ambientK = 0.1;
	float diffuseK = 0.3;
	float specular = 0.5;
	//not 1 - 0
	float shininess = 256.0;


};
Material material;

int main() {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Camera", NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return 1;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Global settings
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	ew::Shader shader("assets/defaultLit.vert", "assets/defaultLit.frag");
	unsigned int brickTexture = ew::loadTexture("assets/brick_color.jpg",GL_REPEAT,GL_LINEAR);

	//light object
	ew::Shader unlitShader("assets/unlit.vert", "assets/unlit.frag");
	ew::Mesh unlitShpereMesh(ew::createSphere(0.2, 10));


	ew::Transform unLitsphereTransfrom[MAX_LIGHTS];
	
	unLitsphereTransfrom[0].position = ew::Vec3(0.0, 1.5, -2.0);
	unLitsphereTransfrom[1].position = ew::Vec3(0.0, 1.5, 2.0);
	unLitsphereTransfrom[2].position = ew::Vec3(2.0, 1.5, 0.0);
	unLitsphereTransfrom[3].position = ew::Vec3(-2.0, 1.5, 0.0);

	//Create cube
	ew::Mesh cubeMesh(ew::createCube(1.0f));
	ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10));
	ew::Mesh sphereMesh(ew::createSphere(0.5f, 64));
	ew::Mesh cylinderMesh(ew::createCylinder(0.5f, 1.0f, 32));

	//Initialize transforms
	ew::Transform cubeTransform;
	ew::Transform planeTransform;
	ew::Transform sphereTransform;
	ew::Transform cylinderTransform;
	planeTransform.position = ew::Vec3(0, -1.0, 0);
	sphereTransform.position = ew::Vec3(-1.5f, 0.0f, 0.0f);
	cylinderTransform.position = ew::Vec3(1.5f, 0.0f, 0.0f);

	//default light colors
	lights[0].position = unLitsphereTransfrom[0].position;
	lights[0].color = ew::Vec3(1.0, 1.0, 1.0);

	lights[1].position = unLitsphereTransfrom[1].position;
	lights[1].color = ew::Vec3(0.0, 1.0, 1.0);

	lights[2].position = unLitsphereTransfrom[2].position;
	lights[2].color = ew::Vec3(1.0, 0.0, 1.0);

	lights[3].position = unLitsphereTransfrom[3].position;
	lights[3].color = ew::Vec3(1.0, 1.0, 0.0);

	resetCamera(camera,cameraController);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		float deltaTime = time - prevTime;
		prevTime = time;

		//Update camera
		camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
		cameraController.Move(window, &camera, deltaTime);

		//RENDER
		glClearColor(bgColor.x, bgColor.y,bgColor.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		shader.setInt("_Texture", 0);
		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		//Draw shapes
		shader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		shader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		shader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		for (int i = 0; i < numberOfLights; i++)
		{
			 
			shader.setVec3("_Lights["+ std::to_string(i)+"].position", lights[i].position);
			shader.setVec3("_Lights["+ std::to_string(i) +"].color", lights[i].color);

		}


		//second light
		shader.setVec3("_Lights[1].position", lights[1].position);
		shader.setVec3("_Lights[1].color", lights[1].color);


		shader.setFloat("_Material.ambientK",material.ambientK);
		shader.setFloat("_Material.diffuseK",material.diffuseK);
		shader.setFloat("_Material.specular",material.specular);
		shader.setFloat("_Material.shininess",material.shininess);

		//ask if i need camera position
		shader.setVec3("cameraPos", camera.position);
		shader.setInt("ifPhong", ifPhongInt);
		shader.setInt("numLights", numberOfLights);
		unlitShader.use();

		unlitShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());


		for (int i = 0; i < numberOfLights; i++)
		{
			unlitShader.setMat4("_Model", unLitsphereTransfrom[i].getModelMatrix());
			unlitShader.setVec3("_Color", lights[i].color);
			unlitShpereMesh.draw();
		}
		
		//second light

		//TODO: Render point lights

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			if (ImGui::CollapsingHeader("Camera")) {
				ImGui::DragFloat3("Position", &camera.position.x, 0.1f);
				ImGui::DragFloat3("Target", &camera.target.x, 0.1f);
				ImGui::Checkbox("Orthographic", &camera.orthographic);
				if (camera.orthographic) {
					ImGui::DragFloat("Ortho Height", &camera.orthoHeight, 0.1f);
				}
				else {
					ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 180.0f);
				}
				ImGui::DragFloat("Near Plane", &camera.nearPlane, 0.1f, 0.0f);
				ImGui::DragFloat("Far Plane", &camera.farPlane, 0.1f, 0.0f);
				ImGui::DragFloat("Move Speed", &cameraController.moveSpeed, 0.1f);
				ImGui::DragFloat("Sprint Speed", &cameraController.sprintMoveSpeed, 0.1f);
				if (ImGui::Button("Reset")) {
					resetCamera(camera, cameraController);
				}
			}
			if (ImGui::CollapsingHeader("material"))
			{
				ImGui::SliderFloat("ambient", &material.ambientK, 0, 1);
				ImGui::SliderFloat("diffuse", &material.diffuseK, 0, 1);
				ImGui::SliderFloat("specular", &material.specular, 0, 1);
				ImGui::SliderFloat("shininess", &material.shininess, 2, 256);
				
			}
			ImGui::SliderInt("number of lights", &numberOfLights, 1, MAX_LIGHTS);
			if (ImGui::Checkbox("Blinn-phong", &ifPhongBool))
			{
				if (ifPhongBool == true)
				{
					ifPhongInt = 1;
				}
				else
				{
					ifPhongInt = 0;
				}

			}
			
			ImGui::Checkbox("orbit", &orbit);
			if (orbit == true)
			{
				for (int i = 0; i < numberOfLights; i++)
				{
					ew::Vec3 nextPos = ew::Cross(ew::Vec3(0, 1, 0), lights[i].position);
					lights[i].position += nextPos * 0.001;
					unLitsphereTransfrom[i].position = lights[i].position;
				}
			}

			for (int i = 0; i < numberOfLights; i++)
			{
				ImGui::PushID(i);
				if (ImGui::CollapsingHeader("light"))
				{
					ImGui::ColorEdit3("light color", &lights[i].color.x);
					ImGui::DragFloat3("light position", &lights[i].position.x);
					unLitsphereTransfrom[i].position = lights[i].position;
				}
				ImGui::PopID();
			}


			ImGui::ColorEdit3("BG color", &bgColor.x);
			ImGui::End();
			
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void resetCamera(ew::Camera& camera, ew::CameraController& cameraController) {
	camera.position = ew::Vec3(0, 0, 5);
	camera.target = ew::Vec3(0);
	camera.fov = 60.0f;
	camera.orthoHeight = 6.0f;
	camera.nearPlane = 0.1f;
	camera.farPlane = 100.0f;
	camera.orthographic = false;

	cameraController.yaw = 0.0f;
	cameraController.pitch = 0.0f;
}

