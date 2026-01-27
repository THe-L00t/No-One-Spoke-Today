#include "Human.h"

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution uid{ 0,100 };

Human::Human()
{
	personal.rationality = uid(dre);
	personal.impulsiveness = uid(dre);
	personal.aggressiveness = uid(dre);
	personal.planning = uid(dre);
	personal.dependency = uid(dre);
	personal.stubbornness = uid(dre);
}
