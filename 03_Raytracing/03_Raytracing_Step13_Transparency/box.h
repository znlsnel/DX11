#pragma once
#include "Square.h"
#include <vector>

namespace hlab 
{
	using namespace std;
	class Box
	{
	public:
		shared_ptr<Square> frontBox, backBox, leftBox, rightBox, topBox, bottomBox;
		std::vector<shared_ptr<Square>> Squares;

		Box(vec3 LeftTopFront, vec3 RightTopFront, vec3 rightBottonFront, vec3 LeftBottomFront, vec3 LeftTopBack, vec3 RightTopBack, vec3 rightBottomBack, vec3 LeftBottomBack)
			//frontBox(LeftTopFront, RightTopFront, rightBottonFront, LeftBottomFront), 
			//backBox(LeftTopBack, RightTopBack, rightBottomBack, LeftBottomBack),
			//leftBox(RightTopBack, LeftTopFront, LeftBottomFront, rightBottomBack), 
			//rightBox(RightTopFront, LeftTopBack, LeftBottomBack, rightBottonFront), 
			//topBox(RightTopBack, LeftTopBack, RightTopFront, LeftTopFront), 
			//bottomBox(LeftBottomFront, rightBottonFront, LeftBottomBack, rightBottomBack)
		{
			frontBox = make_shared<Square>(LeftTopFront, RightTopFront, rightBottonFront, LeftBottomFront);
			backBox = make_shared<Square>(LeftTopBack, RightTopBack, rightBottomBack, LeftBottomBack);
			leftBox = make_shared<Square>(RightTopBack, LeftTopFront, LeftBottomFront, rightBottomBack);
			rightBox = make_shared<Square>(RightTopFront, LeftTopBack, LeftBottomBack, rightBottonFront);
			topBox = make_shared<Square>(RightTopBack, LeftTopBack, RightTopFront, LeftTopFront);
			bottomBox = make_shared<Square>(LeftBottomFront, rightBottonFront, LeftBottomBack, rightBottomBack);

			Squares.push_back(frontBox);
			Squares.push_back(backBox);
			Squares.push_back(leftBox);
			Squares.push_back(rightBox);
			Squares.push_back(topBox);
			Squares.push_back(bottomBox);
			for (auto square : Squares) {
				square->isCubeMap = true;
			}
		}


		// , shared_ptr<Texture> ambTexture, shared_ptr<Texture> difTexture
		void InitBox(vec3 amb, vec3 dif, vec3 spec, float alpha, float reflection) {
			for (auto square : Squares) {
				square->amb = amb;
				square->dif = dif;
				square->spec = spec;
				square->alpha = alpha;
				square->reflection = reflection;
			}
		}

		void InitTexture(shared_ptr<Texture> front, shared_ptr<Texture> back, shared_ptr<Texture> left, shared_ptr<Texture> right, shared_ptr<Texture> top, shared_ptr<Texture> floor) {
			frontBox->difTexture = front;
			frontBox->ambTexture = front;

			backBox->difTexture = back;
			backBox->ambTexture = back;

			leftBox->difTexture = left;
			leftBox->ambTexture = left;

			rightBox->difTexture = right;
			rightBox->ambTexture = right;

			topBox->difTexture = top;
			topBox->ambTexture = top;

			bottomBox->difTexture = floor;
			bottomBox->ambTexture = floor;
		}
		void InitTexture(shared_ptr<Texture> texture)
		{
			for (auto square : Squares) {
				square->difTexture = texture;
				square->ambTexture = texture;
			}
			
		}


		//virtual Hit CheckRayCollision(Ray& ray)
		//{
		//	std::vector<Hit> hits;
		//	for (auto square : Squares) {
		//		Hit tempHit = square->CheckRayCollision(ray);
		//		if (tempHit.d >= 0.f)
		//			hits.push_back(tempHit);
		//	}

		//	Hit Result{-1.f, vec3(0.f), vec3(0.f), vec3(0.f)};
		//	for (auto hit : hits) {
		//		if (hit.d < Result.d || Result.d == -1.f)
		//			Result = hit;
		//	}
		//	return Result;
		//}
	};
}