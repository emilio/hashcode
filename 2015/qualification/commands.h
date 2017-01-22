#ifndef COMMANDS_H
#define COMMANDS_H

#include <iostream>
#include <string>

using namespace std;


class Command {
    size_t extra_duration() { return 0; }
	virtual string toString();
};

class LoadCommand : public Command {
public:
    size_t droneId;
    size_t warehouseId;
	size_t productType;
    size_t count;

	LoadCommand(size_t droneId, size_t warehouseId, size_t productType, size_t count);
	string toString();

    size_t extra_duration() { return 1; }
};

class UnloadCommand: public Command {
	public:
	size_t droneId;
	size_t warehouseId;
	size_t productType;
	size_t count;
	UnloadCommand(size_t droneId, size_t warehouseId, size_t productType, size_t count);
	string toString();

    size_t extra_duration() { return 1; }
};

class DeliverCommand: public Command {
	public:
	size_t droneId;
	size_t orderId;
	size_t productType;
	size_t count;
	string toString();

	DeliverCommand(size_t droneId, size_t orderId, size_t productType, size_t count);

    size_t extra_duration() { return 1; }
};

class WaitCommand: public Command {
	public:
	size_t droneId;
	size_t sleepTurns;
	WaitCommand(size_t droneId, size_t sleepTurns);
	string toString();
};

std::ostream& operator<<(std::ostream& out, const LoadCommand& load);
std::ostream& operator<<(std::ostream& out, const UnloadCommand& unload);
std::ostream& operator<<(std::ostream& out, const DeliverCommand& deliver);
std::ostream& operator<<(std::ostream& out, const WaitCommand& wait);

#endif
