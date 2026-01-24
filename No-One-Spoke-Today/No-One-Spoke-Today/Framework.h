#pragma once
#include "pch.h"
#include "Time.h"

class Framework
{
public:
	Framework();
	~Framework();

	void Init();
	void Loop();
	void Destroy();
private:
	std::unique_ptr<Time> timer;
};

