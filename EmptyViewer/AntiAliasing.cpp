#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glm;
using namespace std;

// -------------------------------------------------
// Global Variables
// -------------------------------------------------
int Width = 512;
int Height = 512;
std::vector<float> OutputImage;
// -------------------------------------------------


class Ray { //class ray
public:
	vec3 origin;
	vec3 direction;

	Ray() : origin(vec3(0)), direction(vec3(0)) {}
	Ray(const vec3& o, const vec3& d) : origin(o), direction(d) {}
};

class Camera { //class camera
public:
	vec3 eye;
	vec3 u, v, w;
	float l, r, b, t, d;
	int nx, ny;

	Camera() {
		eye = vec3(0, 0, 0);
		u = vec3(1, 0, 0);
		v = vec3(0, 1, 0);
		w = vec3(0, 0, 1);
		l = -0.1f;
		r = 0.1f;
		b = -0.1f;
		t = 0.1f;
		d = 0.1f;
		nx = 512;
		ny = 512;
	}

	Ray generateRay(float i, float j) { //change int to float due error
		float u_s = l + (r - l) * (i + 0.5f) / static_cast<float>(nx); 
		float v_s = b + (t - b) * (j + 0.5f) / static_cast<float>(ny);
		vec3 direction = normalize(u * u_s + v * v_s - w * d);
		return Ray(eye, direction);
	}
};

class Plane { //class plane
public:
	vec3 normal;
	float d;
	vec3 ka, kd, ks;
	int specularPower;

	Plane(const vec3& n, float d, const vec3& ka, const vec3& kd, const vec3& ks, int sp)
		: normal(n), d(d), ka(ka), kd(kd), ks(ks), specularPower(sp) {
	}

	bool intersect(const Ray& ray, float& t) {
		float denom = dot(normal, ray.direction);
		if (abs(denom) > FLT_EPSILON) { //possible divison by zero error
			t = -(dot(normal, ray.origin) + d) / denom;
			return (t >= 0);
		}
		return false;
	}
};

class Sphere { //class sphere
public:
	vec3 center;
	float radius;
	vec3 ka, kd, ks;
	int specularPower;

	Sphere(const vec3& c, float r, const vec3& ka, const vec3& kd, const vec3& ks, int sp)
		: center(c), radius(r), ka(ka), kd(kd), ks(ks), specularPower(sp) {
	}

	bool intersect(const Ray& ray, float& t) {
		vec3 oc = ray.origin - center;
		float a = dot(ray.direction, ray.direction);
		float b = 2.0f * dot(oc, ray.direction);
		float c = dot(oc, oc) - radius * radius;
		float discriminant = b * b - 4 * a * c;
		if (discriminant > 0) {
			t = (-b - glm::sqrt(discriminant)) / (2.0f * a);
			return (t >= 0);
		}
		return false;
	}
};

class Light { //class light
public:
	vec3 position;
	vec3 color;

	Light(const vec3& pos, const vec3& col) : position(pos), color(col) {}
};

Light light(vec3(-4, 4, -3), vec3(1, 1, 1)); // White light at position (-4, 4, -3)

vec3 phongShading(const vec3& point, const vec3& normal, const vec3& viewDir, const vec3& lightDir, const vec3& lightColor, const vec3& ka, const vec3& kd, const vec3& ks, int specularPower) {
	
	vec3 ambient = ka * lightColor;

	
	float diff = glm::max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = kd * diff * lightColor;

	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(glm::max(dot(viewDir, reflectDir), 0.0f), static_cast<float>(specularPower));
	vec3 specular = ks * spec * lightColor;

	
	vec3 color = ambient + diffuse + specular;
	return color;
}

class Scene { //class scene
public:
	vector<Plane> planes;
	vector<Sphere> spheres;

	Scene() {
		planes.push_back(Plane(vec3(0.0f, 1.0f, 0.0f), 2.0f, vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0)); // plane located at y = -2 (white)
		spheres.push_back(Sphere(vec3(-4.0f, 0.0f, -7.0f), 1.0f, vec3(0.2f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0)); // s1 (red)
		spheres.push_back(Sphere(vec3(0.0f, 0.0f, -7.0f), 2.0f, vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.5f, 0.5f, 0.5f), 32)); // s2 (green)
		spheres.push_back(Sphere(vec3(4.0f, 0.0f, -7.0f), 1.0f, vec3(0.0f, 0.0f, 0.2f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0)); // s3 (blue)
	}

	bool intersect(const Ray& ray, float& t, vec3& ka, vec3& kd, vec3& ks, int& specularPower, vec3& normal) {
		t = FLT_MAX;
		bool hit = false;
		for (auto& plane : planes) {
			float t_temp;
			if (plane.intersect(ray, t_temp) && t_temp < t) {
				t = t_temp;
				ka = plane.ka;
				kd = plane.kd;
				ks = plane.ks;
				specularPower = plane.specularPower;
				normal = plane.normal;
				hit = true;
			}
		}
		for (auto& sphere : spheres) {
			float t_temp;
			if (sphere.intersect(ray, t_temp) && t_temp < t) {
				t = t_temp;
				ka = sphere.ka;
				kd = sphere.kd;
				ks = sphere.ks;
				specularPower = sphere.specularPower;
				normal = normalize(ray.origin + t * ray.direction - sphere.center);
				hit = true;
			}
		}
		return hit;
	}
};

Scene scene;
Camera camera;

bool isInShadow(const vec3& point, const vec3& lightDir) {
	Ray shadowRay(point + lightDir * 0.001f, lightDir);
	float t;
	vec3 ka, kd, ks, normal;
	int specularPower;
	return scene.intersect(shadowRay, t, ka, kd, ks, specularPower, normal);
}

class GammaCorrection {
public:
	float gamma;

	GammaCorrection(float g = 2.2f) : gamma(g) {}

	vec3 apply(const vec3& color) const {
		return pow(glm::clamp(color, 0.0f, 1.0f), vec3(1.0f / gamma));

	}
};

class AntiAliasing {
public:
	int numSamples;
	AntiAliasing(int samples = 64) : numSamples(samples) {}

	
	vec3 computePixelColor(int i, int j, Camera& camera, Scene& scene,
		Light& light, GammaCorrection& gammaCorrection) {
		vec3 accumulatedColor(0.0f);
		for (int s = 0; s < numSamples; ++s) {
			
			float offsetU = ((float)rand() / RAND_MAX - 0.5f);
			float offsetV = ((float)rand() / RAND_MAX - 0.5f);
			Ray ray = camera.generateRay(i + offsetU, j + offsetV);

			float t;
			vec3 ka, kd, ks, normal;
			int specularPower;
			vec3 color(0.0f);

			if (scene.intersect(ray, t, ka, kd, ks, specularPower, normal)) {
				vec3 hitPoint = ray.origin + t * ray.direction;
				vec3 viewDir = normalize(camera.eye - hitPoint);
				vec3 lightDir = normalize(light.position - hitPoint);
				if (!isInShadow(hitPoint, lightDir)) {
					color = phongShading(hitPoint, normal, viewDir, lightDir,
						light.color, ka, kd, ks, specularPower);
				}
				else {
					color = ka * light.color;
				}
			}
			accumulatedColor += color;
		}
		accumulatedColor /= float(numSamples); //(1/N)
		return gammaCorrection.apply(accumulatedColor);
	}
};

void render()
{
	//Create our image. We don't want to do this in 
	//the main loop since this may be too slow and we 
	//want a responsive display of our beautiful image.
	//Instead we draw to another buffer and copy this to the 
	//framebuffer using glDrawPixels(...) every refresh

	OutputImage.clear();
	OutputImage.reserve(Width * Height * 4); //Increase the capacity of the vector to avoid reallocation
	GammaCorrection gammaCorrection;
	AntiAliasing antiAliasing(64);

	for (int j = 0; j < Height; ++j)
	{
		for (int i = 0; i < Width; ++i)
		{
			// ---------------------------------------------------
			// --- Implement your code here to generate the image
			// --------------------------------------------------

			vec3 color = antiAliasing.computePixelColor(i, j, camera, scene, light, gammaCorrection);
			OutputImage.push_back(color.r);
			OutputImage.push_back(color.g);
			OutputImage.push_back(color.b);
		}

	}
}



void resize_callback(GLFWwindow*, int nw, int nh)
{
	//This is called in response to the window resizing.
	//The new width and height are passed in so we make 
	//any necessary changes:
	Width = nw;
	Height = nh;
	//Tell the viewport to use all of our screen estate
	glViewport(0, 0, nw, nh);

	//This is not necessary, we're just working in 2d so
	//why not let our spaces reflect it?
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, static_cast<double>(Width)
		, 0.0, static_cast<double>(Height)
		, 1.0, -1.0);

	//Reserve memory for our render so that we don't do 
	//excessive allocations and render the image
	OutputImage.reserve(Width * Height * 3);
	render();
}


int main(int argc, char* argv[])
{
	// -------------------------------------------------
	// Initialize Window
	// -------------------------------------------------

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(Width, Height, "OpenGL Viewer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//We have an opengl context now. Everything from here on out 
	//is just managing our window or opengl directly.

	//Tell the opengl state machine we don't want it to make 
	//any assumptions about how pixels are aligned in memory 
	//during transfers between host and device (like glDrawPixels(...) )
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	//We call our resize function once to set everything up initially
	//after registering it as a callback with glfw
	glfwSetFramebufferSizeCallback(window, resize_callback);
	resize_callback(NULL, Width, Height);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// -------------------------------------------------------------
		//Rendering begins!
		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
		//and ends.
		// -------------------------------------------------------------

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		//Close when the user hits 'q' or escape
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
			|| glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}