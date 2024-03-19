#pragma once
#include "ManimRenderer.h"
#include "math\\BoundingBox.h"

namespace manim {
	using namespace math;


	glm::mat4 CreateCameraViewProjectionMatrix(const glm::vec3& cameraPosition,
		const glm::vec3& cameraTarget,
		float fovDegrees,
		float aspectRatio = 1.0f, // Aspect ratio of the viewport
		float nearPlane = 0.01f, // Near clipping plane
		float farPlane = 100.0f) // Far clipping plane
	{
		glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0, 1, 0));
		glm::mat4 projectionMatrix;

		if (fovDegrees == 0) {
			// Orthographic projection
			float orthoWidth = 10.0f; // Define the width for the orthographic projection
			float orthoHeight = orthoWidth / aspectRatio; // Calculate the height based on the aspect ratio
			projectionMatrix = glm::ortho(-orthoWidth / 2, orthoWidth / 2, -orthoHeight / 2, orthoHeight / 2, nearPlane, farPlane);
		}
		else {
			// Perspective projection
			projectionMatrix = glm::perspective(glm::radians(fovDegrees), aspectRatio, nearPlane, farPlane);
		}

		return projectionMatrix * viewMatrix; // Combine the projection and view matrices
	}
	glm::vec3 CatmullRomSpline(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
		// Catmull-Rom spline matrix
		glm::mat4x3 points = glm::mat4x3(p0, p1, p2, p3);
		glm::mat4 basis = glm::mat4(-0.5, 1.5, -1.5, 0.5,
			1.0, -2.5, 2.0, -0.5,
			-0.5, 0.0, 0.5, 0.0,
			0.0, 1.0, 0.0, 0.0);
		glm::vec4 tVec = glm::vec4(t * t * t, t * t, t, 1);
		return points * basis * tVec;
	}

	struct AnimationCurve {


		std::vector<glm::vec2> anchors;

		AnimationCurve() {

		}
		AnimationCurve(std::initializer_list<glm::vec2>ancs) {
			anchors = ancs;
		}
		void add(float x, float y) {
			anchors.emplace_back(x,y);
		}

		void add(const std::string& anchorString) {
			std::istringstream stream(anchorString);
			std::string segment;

			while (std::getline(stream, segment, ';')) {
				std::istringstream segmentStream(segment);
				float key = 0.0f, value = 0.0f;
				// Attempt to parse the key and value from the segment
				if (segmentStream >> key >> value) {
					// If parsing is successful, add them as a new anchor point
					anchors.emplace_back(key, value);
				}
			}
		}


		float evaluate(float t) {
			if (anchors.empty()) {
				return 0.0f; // Return a default value if there are no anchors
			}

			// If t is outside the x range of the anchors, clamp it
			if (t <= anchors.front().x) return anchors.front().y;
			if (t >= anchors.back().x) return anchors.back().y;

			// Find the segment (p0, p1) that encloses t
			glm::vec2 p0 = anchors[0];
			glm::vec2 p1 = anchors[1];

			for (size_t i = 1; i < anchors.size(); ++i) {
				p0 = anchors[i - 1];
				p1 = anchors[i];

				if (t <= p1.x) {
					break; // Found the segment that contains t
				}
			}

			// Perform linear interpolation between p0 and p1
			float segmentT = (t - p0.x) / (p1.x - p0.x); // Calculate the relative position of t within the segment
			return p0.y + segmentT * (p1.y - p0.y); // Interpolate the y value
		}
	};






	class Animation {


		

		glm::u8vec4 pointColor = { 0, 255, 255, 255 }; // Bright Cyan
		glm::u8vec4 lineColor = { 255, 165, 0, 255 }; // Soft Bright Orange

		std::vector<glm::vec3> points;
		std::vector<glm::f32> alphaValues;

		float iTime = 0.0;
		float animationReset = 100;
		float animationEnd = 40;


		glm::ivec2 grid = { 4, 4 };

		glm::i32 maxWidth = 32;


		Bounds2D<glm::f64> gridScale = Bounds2D(0.0 - 0.5,0.0 - 0.5,1.0 - 0.5,1.0-0.5).Scale(glm::dvec2(4,4));


		

		float pointThickness = 0.08*8/float(maxWidth);
		float lineThickness = 0.003;
		float animatedPointSize;


		glm::mat3 tbn = glm::mat3(1.0,0.0,0.0,0.0,0.0,1.0,0.0,1.0,0.0);

		component::EntityView Camera;



		AnimationCurve heightImpact;
		AnimationCurve heightBlend;

		AnimationCurve transparentPoints;
		AnimationCurve visiblePoints;

		AnimationCurve drawnLines;

	public:
		Animation(component::EntityView camera): Animation() {
			Camera = camera;
		}
	

		Animation() {

			float w = maxWidth;


			float lineTime = 32;

			heightImpact.add(6, 0);
			heightImpact.add(lineTime/2, w);
			heightImpact.add(lineTime, w);
			heightImpact.add(lineTime+6, 0);
			heightImpact.add(lineTime+16, w * w);
			heightImpact.add(lineTime+48, w * w);

			drawnLines.add(0, 0);
			drawnLines.add(lineTime/2, w);
			drawnLines.add(lineTime, w);
			drawnLines.add(lineTime+2, 0);
			drawnLines.add(lineTime+16, 0);
			drawnLines.add(lineTime+48, w*w*2);


			heightBlend.add(0, w);
			heightBlend.add(8, 0);
			heightBlend.add(lineTime+48, w*2);

			visiblePoints.add(0, 0.0);
			visiblePoints.add(4, w);
			visiblePoints.add(lineTime, w);
			visiblePoints.add(lineTime+32, w*w);


			transparentPoints.add(0.0, w);

		}

		

		void processCamInput() {

			float sens = 1.0;
			float speed = 1.0;



		}


		void tick(float delta, Renderer& renderer) {

			auto& transform = Camera.get<component::Transform>();
			auto& camera = Camera.get<component::CameraComponent>();

			auto& frustumMat = camera.getFrustumMat();

			

			points.clear();
			alphaValues.clear();




			float t = iTime*4.0;
			



			float _pcount = visiblePoints.evaluate(t);
			float _tpoints = transparentPoints.evaluate(t);
			float _noiseps = heightImpact.evaluate(t);
			float noise_lerp = heightBlend.evaluate(t);

			float visibleLines = drawnLines.evaluate(t);


			grid.x = maxWidth;
			grid.y = _pcount / maxWidth + 1;

			float progress = t / animationEnd;
			animatedPointSize = pointThickness / (glm::clamp(progress, 0.0f, 2.0f)+1.0);

			for (int vertex = 0; vertex < round(_pcount); vertex++) {


				int x = vertex % maxWidth;
				int y = vertex / maxWidth;

				int width = maxWidth - 1;
				

				float opacity = glm::smoothstep(0.0f, _tpoints, _pcount - vertex);
				float height = glm::smoothstep(0.0f, noise_lerp, _noiseps - vertex);



				glm::vec2 horizontal = gridScale.lerp(glm::f64(x) / maxWidth, 0.0);
				glm::vec2 fw = gridScale.lerp(0.0, glm::f64(y) / (maxWidth - 1.0f));

				float perlin = glm::perlin(glm::vec2(x,y)/float(maxWidth)*3.8122f);




				glm::vec3 position{ horizontal + fw, height*perlin*0.6 } ;
				position = tbn * position;

				glm::vec4 clipPos = frustumMat * glm::vec4(position , 1.0);
				clipPos /= clipPos.w;

				points.emplace_back(clipPos);
				alphaValues.push_back(opacity);

			}
			
			
			if (t > glm::f32(animationReset)) {
				iTime = 0.0f;
			}

			iTime += delta;
			submit(renderer, visibleLines);
		}



		void submit(Renderer& renderer, size_t numLines) {
			if (!points.empty()) {
				const size_t pointsCount = points.size();
				size_t linesDrawn = 0;

				for (size_t y = 0; y < grid.y; y++) {
					for (size_t x = 0; x < grid.x; x++) {
						size_t indexA = x + y * grid.x;
						if (indexA >= pointsCount) continue; 
						glm::vec3 pointA = points[indexA];
						auto alpha = alphaValues[indexA];
						renderer.drawDot(pointA, animatedPointSize /(pointA.z), pointColor.r, pointColor.g, pointColor.b, alpha * 255.0f);

						if (linesDrawn >= numLines) continue;

						// Attempt to draw a line to the right
						size_t indexB = x + 1 + y * grid.x;
						if (x != grid.x - 1 && indexB < pointsCount) {
							renderer.drawLine(pointA, points[indexB], lineColor.r, lineColor.g, lineColor.b, lineThickness);
							linesDrawn++;
							if (linesDrawn >= numLines) continue;
						}

						size_t indexDown = x + (y + 1) * grid.x;
						if (y != grid.y - 1 && indexDown < pointsCount) {
							renderer.drawLine(pointA, points[indexDown], lineColor.r, lineColor.g, lineColor.b, lineThickness);
							linesDrawn++;
							if (linesDrawn >= numLines) continue;
						}

					}
				}
			}
		}



	};
}
