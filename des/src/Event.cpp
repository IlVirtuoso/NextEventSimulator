#include "Event.hpp"

int Event::DeletedNodes = 0;

int Event::GeneratedNodes = 0;
bool Event::operator==(Event& oth) {
    return Name == oth.Name&& OccurTime == oth.OccurTime
    && Type == oth.Type;
}
