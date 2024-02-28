#pragma once
#include <type_traits>
#include <glm/gtc/type_ptr.hpp>

#include "IRenderPipeline.h"
#include "IRenderSystem.h"

#include "components/CameraComponent.h"
#include "scene/Scene.h"

#include "Gizmos.h"
#include "rpdef.h"
#include "opengl/Mesh.h"
#include "opengl/ShaderFactory.h"
#include "opengl/VertexArrayObject.h"
#include "rpdef.h"
#include "utility/Statistic.h"

namespace renderer
{
	using namespace graphics;

	


	struct RenderPipeline :public IRenderPipeline
	{
		using fbo = FrameBufferObject;
		using fbo_shared = std::shared_ptr<FrameBufferObject>;
		using Entity = component::Entity;


		scene::SceneClass* m_scene = nullptr;

		std::shared_ptr<component::CameraComponent::System> cameraSystem = nullptr;

		std::shared_ptr<shader::ComputeShader> lightingPass;
		std::vector<fbo_shared> frameBuffers;

		fbo_shared mainFramebuffer;


		Entity renderSystemInfo;




		void addFrameBuffer(fbo_shared frameBuffer)
		{
			frameBuffers.emplace_back(std::move(frameBuffer));
		}

	public:
		fbo_shared GetRenderTexture(const std::string& str) override
		{
			fbo_shared& fbo = *std::ranges::find_if(frameBuffers.begin(), frameBuffers.end(), [str](const fbo_shared& frameBuffer) {
				return frameBuffer->GetName() == str;
				});
			return fbo;
		}

		RenderPipeline() = default;

		void Initialize(scene::SceneClass* scene)override
		{
			m_scene = scene;
			renderSystemInfo = m_scene->CreateEntity();
			cameraSystem = scene->componentSystems.get_shared<component::CameraComponent::System>();
			renderSystemInfo.emplace<CameraBuffer>(&cameraSystem->GetMatrixBuffer());
			renderSystemInfo.emplace<CurrentCamera>(nullptr, entt::null);


			Sampler defaultSampler;
			defaultSampler.minFilter = MinFilter::NEAREST;
			defaultSampler.magFilter = MagFilter::NEAREST;
			defaultSampler.wrapR = WrapMode::CLAMP_TO_EDGE;
			defaultSampler.wrapT = WrapMode::CLAMP_TO_EDGE;
			defaultSampler.wrapS = WrapMode::CLAMP_TO_EDGE;

			Sampler shadowSampler;
			shadowSampler.minFilter = MinFilter::NEAREST;
			shadowSampler.magFilter = MagFilter::NEAREST;
			shadowSampler.wrapR = WrapMode::CLAMP_TO_EDGE;
			shadowSampler.wrapT = WrapMode::CLAMP_TO_EDGE;
			shadowSampler.wrapS = WrapMode::CLAMP_TO_EDGE;
			shadowSampler.compareFunc = CompareFunc::LESS;
			shadowSampler.compareMode = CompareMode::COMPARE_REF_TO_TEXTURE;

			
			auto& transform = m_scene->screen.GetWindowSize();

			fbo* fboPtr = new fbo();
			fboPtr->initialize("GeometryTarget", 0, 0);
			fboPtr->emplace<component::FBOResizeComponent>(fboPtr, transform);

			fboPtr->EmplaceTexture(FrameBufferTarget::Albedo, TextureFormat::RGBA8, TextureTarget::Texture2D, defaultSampler);
			fboPtr->EmplaceTexture(FrameBufferTarget::WorldNormal, TextureFormat::RGB10_A2, TextureTarget::Texture2D, defaultSampler);
			fboPtr->EmplaceTexture(FrameBufferTarget::WorldPosition, TextureFormat::RGBA16, TextureTarget::Texture2D, defaultSampler);
			fboPtr->EmplaceTexture(FrameBufferTarget::RoughnessMetallicSpecular, TextureFormat::RGBA8, TextureTarget::Texture2D, defaultSampler);
			fboPtr->EmplaceTexture(FrameBufferTarget::Depth, TextureFormat::DepthComponent24, TextureTarget::Texture2D, defaultSampler);

			addFrameBuffer(std::shared_ptr<fbo>(fboPtr));

			

			fboPtr = new fbo();
			fboPtr->initialize("LightingTarget", 0, 0);
			fboPtr->emplace<component::FBOResizeComponent>(fboPtr, transform);
			fboPtr->EmplaceTexture(FrameBufferTarget::Albedo, TextureFormat::RGBA8, TextureTarget::Texture2D, defaultSampler);
			addFrameBuffer(std::shared_ptr<fbo>(fboPtr));


			fboPtr = new fbo();
			fboPtr->initialize("ShadowTarget", 0, 0);
			fboPtr->emplace<component::FBOResizeComponent>(fboPtr, transform);
			fboPtr->EmplaceTexture(FrameBufferTarget::Depth, TextureFormat::DepthComponent16, TextureTarget::Texture2D, shadowSampler);
			addFrameBuffer(std::shared_ptr<fbo>(fboPtr));




			mainFramebuffer = m_scene->screen.GetMainFrameBuffer();

			lightingPass =
				shader::ShaderFactory::MakeComputeShader(
					"$Resources\\ComputeShaders\\LightingPass.glsl");
			


		}


		glm::mat4 makeDirectionalLightMatrix(glm::mat4 camMat, glm::vec3 camPos)
		{
			return camMat;
			/*
			using math::AABB32;
			Gizmos::Instance().DrawCube(lightTransform->model());

			glm::vec3 lightPos = lightTransform->position();

			float depth = 1000.0f;
			float boxSize = 32.0f;

			math::Bounds2D<float> bounds(-boxSize, -boxSize, boxSize, boxSize);

			glm::vec3 size(boxSize, boxSize, depth);
			glm::vec3 worldCenter = camPos + glm::mat3_cast(lightTransform->rotation()) * glm::vec3(0, 0, 1) * 100.0f;

			glm::mat4 lightMatrix = glm::lookAt(-lightTransform->position() * depth + worldCenter, worldCenter, glm::vec3(0, 1, 0));

			AABB32 testBox(-0.5f, 0.5f);

			float newNear = 0.0f;
			float newFar = depth;

			glm::mat4 testLightSpaceMatrix = glm::ortho(bounds.x0, bounds.x1, bounds.y0, bounds.y1, newNear, newFar) * lightMatrix;

			AABB32 scaledTestBox = AABB32::Transform(testBox, glm::inverse(testLightSpaceMatrix));
			glm::vec3 scale = 1.0f / scaledTestBox.GetExtends();
			AABB32 box = AABB32(-size * 0.5f, size * 0.5f);
			box.Scale(scale);

			return glm::ortho(bounds.x0, bounds.x1, bounds.y0, bounds.y1, newNear, newFar) * lightMatrix;
			*/
		}

		void LogMatrix(const glm::mat4& matrix) {
			std::cout << "Matrix:" << std::endl;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					std::cout << matrix[i][j] << "\t";
				}
				std::cout << std::endl;
			}
		}


		void Render()override
		{
			glEnable(GL_DEPTH_TEST);
			glm::mat4 lightMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(50));

			//todo dispatch lighting shader
			auto& shadowMap = *GetRenderTexture("ShadowTarget");
			auto& geometry = *GetRenderTexture("GeometryTarget");
			auto& lighting = *GetRenderTexture("LightingTarget");
			auto& blit = *mainFramebuffer;
			const Buffer& cameraBuffer = *renderSystemInfo.get<CameraBuffer>();

			geometry.ClearColor(30, 136, 229, 255);
			lighting.ClearColor();

			blit.ClearColor();

			

			size_t i = 0;
			for (const auto camera : cameraSystem->GetCameras())
			{
				cameraSystem->GetMatrixBuffer().SetBufferRange(static_cast<int32_t>(sizeof(component::CameraComponent::MatrixData) * i++));
				renderSystemInfo.get<CurrentCamera>() = camera;
				component::EntityView view = renderSystemInfo.view();
				
				m_scene->renderSystems.ForEach([&](IRenderSystem* sys) {
					sys->CullCamera(view);
					});

				m_scene->renderSystems.ForEach([&, this, camera](IRenderSystem* sys) {
					geometry.Bind();
					sys->RenderColor(renderSystemInfo);
					});
			}
			cameraBuffer.SetBufferRange(static_cast<int32_t>(sizeof(component::CameraComponent::MatrixData) * cameraSystem->GetMainCameraIndex()));


			lightingPass->BindSamplerTexture("albedoTexture", geometry[graphics::FrameBufferTarget::Albedo], 0);
			lightingPass->BindSamplerTexture("worldNormalTexture", geometry[graphics::FrameBufferTarget::WorldNormal], 1);
			lightingPass->BindSamplerTexture("worldPositionTexture", geometry[graphics::FrameBufferTarget::WorldPosition], 2);
			lightingPass->BindSamplerTexture("rsmTexture", geometry[graphics::FrameBufferTarget::RoughnessMetallicSpecular], 3);
			lightingPass->BindSamplerTexture("depthTexture", geometry[graphics::FrameBufferTarget::Depth], 4);
			//lightingPass->BindSamplerTexture("shadowMap", geometry[graphics::FrameBufferTarget::Depth], 5);
			lightingPass->UniformMatrix4fv("lightMatrix", 1, false, glm::value_ptr(lightMatrix));

			lightingPass->BindBufferRange("CameraSSBO", cameraBuffer);

			lighting[FrameBufferTarget::Albedo].BindToImageUnit(0, WriteAccess::Writeonly);

			lightingPass->DispatchCompute(blit.width() / 16, blit.height() / 16, 1);
			
			if (true)
			{
				lighting.Blit(blit, FrameBufferTarget::Albedo
				);
				geometry.Blit(blit, FrameBufferTarget::Depth
				);
			}
			if(m_scene->inputSystem.IsHitBegin(GLFW_KEY_SPACE))
				DebugAlbedoOutput(&lighting[FrameBufferTarget::Albedo]);
			

			m_scene->screen.GetMainFrameBuffer()->Bind();

			glLineWidth(3);
			Gizmos::Instance().Render(cameraBuffer);
			Gizmos::Instance().Reset();
			glLineWidth(1);
			//DrawDebugTriangleWithMesh(blit);
		}

		void DebugAlbedoOutput(const Texture* text)
		{
			std::vector<float> data;
			data.resize(text->getWidth()*text->getHeight() * sizeof(float)* 4);
			text->GetTexture2DSubImage(*text, data, ExternalTextureFormat::Red, DataType::float32_t);


			std::array<size_t, 256> dist{ 0 };

			size_t modus = 0;
			size_t modus_value = 0;
			for(float f: data){

				if (f > 0.001) {
					dist[static_cast<size_t>(f * 255.0f)]++;
				}
			}
			std::vector<statistic::Point> points;

			for (size_t i = 0; i < dist.size(); i++){
				modus = dist.size() > modus_value ? i : modus;
					modus_value = modus == i ? dist.size() : modus;
					points.emplace_back(i, dist[i] / std::max(i, 1ull));
			}

			const double mean = statistic::mean(points);
			const double variance = statistic::variance(points, mean);
			const std::vector<double> values = statistic::flatten(points);
			const auto samples = statistic::take_medians(values, {0,1,10,30});


			auto format = [](double Mean, double Variance, size_t n, const std::vector<std::pair<double, double>>& values) {
				std::stringstream ss;

				ss << "Mean = " << Mean << "\n";
				ss << "Variance = " << Variance << "\n";
				ss << "Standard Derivation = " << sqrt(Variance) << "\n";
				ss << "N =  " << n << "\n\n";

				if (!values.empty()) {
					ss << "Values:\n";
					for (const auto& [x, y] : values) {
						ss << static_cast<size_t>(x) << "%: " << static_cast<size_t>(y) << "\n";
					}
				}
				else {
					ss << "No values provided.\n";
				}

				return ss.str();
				};

			std::cout << format(mean, variance, values.size(), samples) << std::endl << std::endl;

			std::string svg = statistic::generateExcel(points);

			asset::WriteFile(svg, "C:\\Users\\kleme\\Downloads\\SVG\\stastistic.csv");
			std::cout << std::endl;
		}
		void DrawDebugTriangleWithMesh(const FrameBufferObject& fbo)
		{
			const char* vertexShaderSource = "#version 330 core\n"
				"layout (location = 0) in vec3 aPos;\n"
				"void main()\n"
				"{\n"
				"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
				"}\0";

			// Fragment Shader source code
			const char* fragmentShaderSource = "#version 330 core\n"
				"out vec4 FragColor;\n"
				"void main()\n"
				"{\n"
				"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
				"}\n\0";



			unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glCompileShader(vertexShader);

			unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
			glCompileShader(fragmentShader);

			unsigned int shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, vertexShader);
			glAttachShader(shaderProgram, fragmentShader);
			glLinkProgram(shaderProgram);

			glDeleteShader(vertexShader); 
			glDeleteShader(fragmentShader);

			std::vector<float> vertices = {
				-0.5f, -0.5f, 0.0f, 
				 0.5f, -0.5f, 0.0f, 
				 0.0f,  0.5f, 0.0f  
			};

			 std::vector<unsigned int> indices = { 
				0, 1, 2
			};



			shader::useShaderProgram(shaderProgram);
			glUseProgram(shaderProgram);

			/*std::vector<LayoutDescriptor> layout{
				{VertexArrayObject::Position, DataType::float32_t, 0, LayoutDescriptor::AttributeType::Float}
			};*/

			fbo.Bind();

			if(false)
			{
				unsigned int VAO, VBO, EBO;


				shader::useShaderProgram(shaderProgram);//this is needed 
				glUseProgram(shaderProgram);

				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);
				glGenBuffers(1, &EBO);

				glBindVertexArray(VAO);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*3, vertices.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.size()*sizeof(int), indices.data(), GL_STATIC_DRAW);

				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glViewport(0, 0, 1024, 1024);

				glBindVertexArray(0);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

			}else
			{
				/*auto mesh = Mesh::Create(12, layout);
				mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::None);
				mesh->SetIndexData(indices);
				mesh->SetVertexData(vertices);
				mesh->Sync();
				mesh->Draw(std::numeric_limits<uint32_t>::max());*/
			}

			





		}
		void DrawDebugTriangle()
		{

			const char* vertexShaderSource = "#version 330 core\n"
				"layout (location = 0) in vec3 aPos;\n"
				"void main()\n"
				"{\n"
				"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
				"}\0";

			// Fragment Shader source code
			const char* fragmentShaderSource = "#version 330 core\n"
				"out vec4 FragColor;\n"
				"void main()\n"
				"{\n"
				"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
				"}\n\0";



			unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
			glCompileShader(vertexShader);
			// Check for shader compile errors
			// ...

			// Fragment shader
			unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
			glCompileShader(fragmentShader);

			unsigned int shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, vertexShader);
			glAttachShader(shaderProgram, fragmentShader);
			glLinkProgram(shaderProgram);
			// Check for linking errors
			// ...

			glDeleteShader(vertexShader); // the shaders can be deleted after linking
			glDeleteShader(fragmentShader);

			// Define vertices for the triangle
			float vertices[] = {
				-0.5f, -0.5f, 0.0f, // left  
				 0.5f, -0.5f, 0.0f, // right 
				 0.0f,  0.5f, 0.0f  // top   
			};

			unsigned int indices[] = { // Note that we start from 0!
				0, 1, 2 // First Triangle
			};

			// Create reference containers for the Vartex Array Object, the Vertex Buffer Object, and the Element Buffer Object
			unsigned int VAO, VBO, EBO;


			shader::useShaderProgram(shaderProgram);//this is needed 
			glUseProgram(shaderProgram);

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			// Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			// Note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glViewport(0, 0, 1024, 1024);

			glBindVertexArray(0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);





			
		}

	};



}
