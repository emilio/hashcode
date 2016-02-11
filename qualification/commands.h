#ifndef COMMANDS_H
#define COMMANDS_H

#include <iostream>
#include <string>

using namespace std;

class Commands{
	public:
	virtual string toString();	
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

#endif COMMANDS_H
