#pragma once
#include <cassert>
#include "Math.h"
#include <iostream>

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			// Reflectivity ( cd * kd ) / PI == Lambert Diffuse Color
			return { (cd * kd) / dae::PI };
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			// Reflectivity ( cd * kd ) / PI == Lambert Diffuse Color
			return { (cd * kd ) /  dae::PI };
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction 
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{

			Vector3 reflect{ l - ( 2 * ( Vector3::Dot(n, l) ) * n ) };
			reflect.Normalized();

			float cosAngle{ std::max(0.f, Vector3::Dot(reflect, v)) };

			// Do a max to avoid negative values for the angle
			float specularRefl{ ks * (powf(cosAngle, exp)) };
			
			// Phong specular reflection
			return ColorRGB{ specularRefl, specularRefl , specularRefl };

		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			

			float result1{ 1 - (Vector3::Dot(h, v) ) };
			float dotResult{ powf(result1, 5.f) };

			return { ColorRGB{ f0.r + ( (1 - f0.r) * dotResult ) , f0.g + ( (1 - f0.g) * dotResult ) , f0.b + ( (1 - f0.b) * dotResult ) } };
			
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			float alpha{ roughness * roughness };
			float alphaSquared = alpha * alpha;
			float cosThetaH = Vector3::Dot(n, h);
			float cosThetaHSquared = cosThetaH * cosThetaH;
			float alphaPlus1{ alphaSquared - 1 };

			float denominator{ cosThetaHSquared * alphaPlus1 + 1 };
			denominator *= denominator;
			denominator *= dae::PI;

			return alphaSquared / denominator;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			float dotProduct{ Vector3::Dot(n, v) };

			return { dotProduct / (dotProduct * ( 1 - roughness)) + roughness };
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			// For direct lighting
			float alpha{ roughness * roughness };
			float k{ powf(alpha + 1, 2) / 8.f};

			// To get correct approximation -> Use it for shadowing light and the masking light
			return GeometryFunction_SchlickGGX(n, v, k) * GeometryFunction_SchlickGGX(n, l, k);
		}

	}
}