#include "Human.h"

std::random_device rd;
std::default_random_engine dre;
std::uniform_int_distribution uid{ 0,100 };

Human::Human()
{
	state.rationality = uid(dre);
	state.impulsiveness = uid(dre);
	state.aggressiveness = uid(dre);
	state.planning = uid(dre);
	state.dependency = uid(dre);
	state.stubbornness = uid(dre);
}
