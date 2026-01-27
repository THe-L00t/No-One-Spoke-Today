#pragma once
#include "pch.h"

struct State {
	int rationality;		// 이성 : 판단을 이성적으로 하려는 경향
	int impulsiveness;	// 충동 : 즉각 반응하는 성향	
	int aggressiveness;	// 폭력 : 공격적 해결 성향
	int planning;			// 계획 : 장기적 사고 성향
	int dependency;		// 의존 : 타인/구조에 기대려는 성향
	int stubbornness;		// 완고 : 생각을 바꾸지 않으려는 성향
};


class Human
{
public:
	Human();

private:
	State state;
};

