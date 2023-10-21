#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{	

			Vector3 sphereToRay{ ray.origin - sphere.origin };

			// Calculate Discriminant to see if there is a full Intersection ( Discriminant > 0 )
			float a{ Vector3::Dot(ray.direction, ray.direction) };
			float b{ Vector3::Dot((2 * ray.direction), sphereToRay) };
			float c{ (Vector3::Dot(sphereToRay, sphereToRay)) - (sphere.radius * sphere.radius) };

			float discriminant{ (b * b) - (4 * a * c) };
			if (discriminant > 0)
			{
				// FULL INTERSECTION OF THE RAY ( 2 intersection points)
				float discSquared{ sqrt(discriminant) };
				float newT{ (-b - discSquared) / (2 * a) };
				if (newT < ray.min)
				{
					// T behind the tMin -> use the + formula
					newT = (-b + discSquared) / (2 * a);
				}

				// Check if it is in [tMin, tMax]	
				if (newT >= ray.min && newT <= ray.max)
				{
					// Valid Range
					if (ignoreHitRecord == false)
					{
						// Check if smaller than the previous t saved
						if (hitRecord.t >= newT)
						{
							// Calculate intersection point
							hitRecord.origin = ray.origin + (newT * ray.direction);
							hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
							hitRecord.t = newT;
							hitRecord.didHit = true;
							hitRecord.materialIndex = sphere.materialIndex;
						}
					}				
					return true;
					
				}
			}
			
			// No intersection or not a full intersection of the ray
			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 toOrigin{ plane.origin - ray.origin };

			float t{ Vector3::Dot(toOrigin, plane.normal) / Vector3::Dot(ray.direction, plane.normal) };
			if (t > ray.min && t < ray.max)
			{
				// t inside [tMin, tMax] from the ray
				if (ignoreHitRecord == false)
				{				
					if (hitRecord.t >= t)
					{
						// Calculate the intersection point 
						hitRecord.origin = ray.origin + (t * ray.direction);

						hitRecord.normal = plane.normal;
						hitRecord.t = t;
						hitRecord.didHit = true;
						hitRecord.materialIndex = plane.materialIndex;
					}
				}
				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			
			// 1º Check if the ray hit the triangle plane
			//..... Calculate normal with triangle edges 
			Vector3 triangleNormal{ Vector3::Cross((triangle.v1 - triangle.v0), (triangle.v2 - triangle.v0)).Normalized() };
			
			float angleNormalRay{ Vector3::Dot(triangleNormal, ray.direction) };
			if (AreEqual(angleNormalRay, 0.f))
				return false;	// If 0 means the ray is perpendicular to the normal so we dont actually see the triangle

			if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
			{
				if (!ignoreHitRecord)
				{
					if (angleNormalRay > 0.f)
						return false;	// Viewing from the front ( Not hitting )
				}
				else
				{
					// If calculating shadows we use the opposite cullMode
					if (angleNormalRay < 0.f)
						return false;	
				}
			}
			if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
			{

				if (!ignoreHitRecord)
				{
					if (angleNormalRay < 0.f)
						return false;	// Viewing from the back ( Not hitting )
				}
				else
				{
					// If calculating shadows we use the opposite cullMode
					if (angleNormalRay > 0.f)
						return false;
				}
			}

			// 2º Where does the ray hit the plane ? 
			Vector3 toPlane{ triangle.v0 - ray.origin };

			//.... Calculate the distance of the vector ( t )
			float t{ ( Vector3::Dot(toPlane, triangleNormal) ) / ( Vector3::Dot(ray.direction, triangleNormal) ) };

			// .... Check if t within range
			if (t < ray.min || t > ray.max)
				return false;	// Not in range

			//.... In range -> Calculate where does intersect
			Vector3 intersectPoint{ ray.origin + (ray.direction * t) };


			// 3º Intersection point inside the triangle? 
			//.... Check for every edge from the triangle if the point is on the right side
			Vector3 edge{};
			Vector3 toPoint{};		// Used to determine if intersect point in the right side of triangle

			// Doing this instead of vector is more optimal
			edge = triangle.v1 - triangle.v0;
			toPoint = intersectPoint - triangle.v0;
			if ((Vector3::Dot(Vector3::Cross(edge, toPoint), triangleNormal)) < 0)
				return false;

			edge = triangle.v2 - triangle.v1;
			toPoint = intersectPoint - triangle.v1;
			if ((Vector3::Dot(Vector3::Cross(edge, toPoint), triangleNormal)) < 0)
				return false;

			edge = triangle.v0 - triangle.v2;
			toPoint = intersectPoint - triangle.v2;
			if ((Vector3::Dot(Vector3::Cross(edge, toPoint), triangleNormal)) < 0)
				return false;

				
			// If here means that intersectPoint is in the right side of the triangle 
			if (ignoreHitRecord == false)
			{
				if (hitRecord.t >= t)
				{
					hitRecord.origin = intersectPoint;
					hitRecord.normal = triangleNormal;
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = triangle.materialIndex;
				}
			}


			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			bool didHit{ false };
			for (size_t index{ 0 }; index < mesh.indices.size(); index += 3)
			{
				// V0 , V1 , V2 and normal
				Triangle triangle{ mesh.transformedPositions[mesh.indices[index]], mesh.transformedPositions[mesh.indices[index + 1]],
				mesh.transformedPositions[mesh.indices[index + 2]],  mesh.transformedNormals[index / 3] };
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				didHit = HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord);		
				if (ignoreHitRecord)
					return didHit; // Return the first hit
			}

			return didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if (light.type == LightType::Directional)
			{
				// Directional light doesn't have an origin
				return light.direction;
			}
	
			return light.origin - origin;
		}

		// How much light reaches a surface ?
		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if (light.type == LightType::Directional)
				return { light.color * light.intensity };

			// If it is a point light we need to take into account the surface area 
			float distanceSquared = (light.origin - target).SqrMagnitude();
			return { light.color * (light.intensity / distanceSquared) };
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}