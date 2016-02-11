#ifndef COMMANDS_H
#define COMMANDS_H

#include <iostream>
#include <string>

using namespace std;

class Commands{
};

class Load:Commands{
	public:
	int droneId;
	int warehouseId;
	int productType;
	int count;
	Load(int droneId, int warehouseId, int productType, int count);
};

class Unload:Commands{
	public:
	int droneId;
	int warehouseId;
	int productType;
	int count;
	Unload(int droneId, int warehouseId, int productType, int count);
};

class Deliver:Commands{
	public:
	int droneId;
	int orderId;
	int productType;
	int count;
	Deliver(int droneId, int orderId, int productType, int count);
};

class Wait:Commands{
	public:
	int droneId;
	int sleepTurns;	
	Wait(int droneId, int sleepTurns);
};

std::ostream& operator<<(std::ostream& out, const Load& load);
std::ostream& operator<<(std::ostream& out, const Unload& unload);
std::ostream& operator<<(std::ostream& out, const Deliver& deliver);
std::ostream& operator<<(std::ostream& out, const Wait& wait);
#endif COMMANDS_H