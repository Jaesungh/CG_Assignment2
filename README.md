# Assignment2

I have implemented Phong Model, Gamma correction, and Antialiasing in this assignment. 

## Results

### Phong Model : 

![image](https://github.com/user-attachments/assets/af64a865-df84-4c45-af74-ba9a049dc0c7)

With Phong Model, I followed material parameters for each object :

- Plane P : ka = (0.2, 0.2, 0.2), kd = (1, 1, 1), ks = (0, 0, 0), with specular
power 0.
– Sphere S1: ka = (0.2, 0, 0), kd = (1, 0, 0), ks = (0, 0, 0), with specular
power 0.
– Sphere S2: ka = (0, 0.2, 0), kd = (0, 0.5, 0), ks = (0.5, 0.5, 0.5), with
specular power 32.
– Sphere S3: ka = (0, 0, 0.2), kd = (0, 0, 1), ks = (0, 0, 0), with specular
power 0.

```c++
		planes.push_back(Plane(vec3(0.0f, 1.0f, 0.0f), 2.0f, vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0)); // plane located at y = -2 (white)
		spheres.push_back(Sphere(vec3(-4.0f, 0.0f, -7.0f), 1.0f, vec3(0.2f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0)); // s1 (red)
		spheres.push_back(Sphere(vec3(0.0f, 0.0f, -7.0f), 2.0f, vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.5f, 0.5f, 0.5f), 32)); // s2 (green)
		spheres.push_back(Sphere(vec3(4.0f, 0.0f, -7.0f), 1.0f, vec3(0.0f, 0.0f, 0.2f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0)); // s3 (blue)
```

Light source at (−4, 4, −3), emitting white light with unit intensity and no falloff.

```c++
Light light(vec3(-4, 4, -3), vec3(1, 1, 1)); // White light at position (-4, 4, -3)
```
Model shadows from the point light using shadow rays and small offset 0.001f is to ensure the shadow ray above the surface to avoid self-intersection 

```c++
bool isInShadow(const vec3& point, const vec3& lightDir) {
	Ray shadowRay(point + lightDir * 0.001f, lightDir);
	float t;
	vec3 ka, kd, ks, normal;
	int specularPower;
	return scene.intersect(shadowRay, t, ka, kd, ks, specularPower, normal);
}
```


### Gamma Correction : 

Added Gamma Correction from previous model (Phong Model). 

![image](https://github.com/user-attachments/assets/3e8d4ddc-c33d-4e00-aea1-0c0323842cd0)

It performs gamma correction with γ = 2.2

```c++
class GammaCorrection {
public:
	float gamma;
	GammaCorrection(float g = 2.2f) : gamma(g) {}
	vec3 apply(const vec3& color) const {
		return pow(glm::clamp(color, 0.0f, 1.0f), vec3(1.0f / gamma));
	}
};
```

And applied gamma correction to render() :

```c++
color = gammaCorrection.apply(color);
```

### Anti Aliasing :

As you can tell from image from Gamma correction and Phong model, the images contain "jaggies" at the boundaries of the sphere and even at the boundaries of shadow of spheres. By implementing Anti Aliasing, the "jaggies" will be removed.

![image](https://github.com/user-attachments/assets/83ba5d99-e29e-40be-9c1c-9f7a5e2aa97c)

I fulfilled the requirement of generating N = 64 samples of the image within each pixel, by generating N random eye rays within each pixel. Also, for each pixel, I applied a box filter with amplitude 1/N and support equal to the extents of the “pixel rectangle” to estimate the pixel value in following codes : 

```c++
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
```

## Compilation and Run Instruction 

Tested on : Visual Studio 2022

In this assignment, I made them in a single project file. So, when compiling and running the project, you need to make exceptions of the source files to the project to compile & run. 

![프로젝트 제외1](https://github.com/user-attachments/assets/faa5eecd-e1f2-4077-a97e-9fdc88e96fb3)

![프로젝트 제외2](https://github.com/user-attachments/assets/cafcb6c1-6bbe-41f7-926d-a4d3ccbcead2)

After making files exception,

-Build- Build Solution (F7)

-Compile- Start debugging (F5)

When you see this error,

![image](https://github.com/user-attachments/assets/9e1d3b6d-3d5b-4a5f-98ac-8d9b662a00bf)

https://www.glfw.org/download.html

Download GLFW according to your system ![image](https://github.com/user-attachments/assets/925d03fa-998d-4310-ba7e-dec12c83defe)

And insert glfw.dll file to /bin folder after running solution and rest of the files (glfw3.lib & glfw3_mt.lib & glfw3dll.lib to /lib folder)

## Short description :

In AntiAliasing.cpp :

It is added to increase the capacity of the vector to avoid reallocation

```c++
OutputImage.reserve(Width * Height * 4)
```

For Ray generateRay, it is changed to float from int due to error
```c++
	Ray generateRay(float i, float j) { 
		float u_s = l + (r - l) * (i + 0.5f) / static_cast<float>(nx); 
		float v_s = b + (t - b) * (j + 0.5f) / static_cast<float>(ny);
		vec3 direction = normalize(u * u_s + v * v_s - w * d);
		return Ray(eye, direction);
	}
```


## References :

https://learnopengl.com/

https://github.com/ADoublePlus/Ray-Tracing

https://github.com/genericalexacc/RayTracing

https://raytracing.github.io/

https://www.youtube.com/watch?v=xnVJ84BpnFs

Assistant from Github Copilot

