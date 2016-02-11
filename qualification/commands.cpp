#include "commands.h"

using namespace std;


string LoadCommand::toString(){
	string ms;
	ms =""+ std::to_string(droneId)+  " L " + std::to_string(warehouseId) + " " + std::to_string(productType) + " " + std::to_string(count);
	return ms;
}

string UnloadCommand::toString(){
	string ms;
	ms ="" +std::to_string(droneId)+  " L " + std::to_string(warehouseId) + " " + std::to_string(productType) + " " + std::to_string(count);
	return ms;
}

string DeliverCommand::toString(){
	string ms;
	ms =""+std::to_string(droneId) + " D " + std::to_string(orderId) + " " + std::to_string(productType) + " " + std::to_string(count);
	return ms;
}
	
	
	
LoadCommand::LoadCommand(size_t droneId, size_t warehouseId, size_t productType, size_t count) : droneId(droneId), warehouseId(warehouseId), productType(productType), count(count) {
}

UnloadCommand::UnloadCommand(size_t droneId, size_t warehouseId, size_t productType, size_t count) : droneId(droneId), warehouseId(warehouseId), productType(productType), count(count) {
}

DeliverCommand::DeliverCommand(size_t droneId, size_t orderId, size_t productType, size_t count) : droneId(droneId), orderId(orderId), productType(productType), count(count) {
}

WaitCommand::WaitCommand(size_t droneId, size_t sleepTurns) : droneId(droneId), sleepTurns(sleepTurns){

}

std::ostream& operator<<(std::ostream& out, const LoadCommand& load) {
	out << load.droneId << " L " << load.warehouseId << " " << load.productType << " " << load.count;
	return out;
}

std::ostream& operator<<(std::ostream& out, const UnloadCommand& unload) {
	out << unload.droneId << " U " << unload.warehouseId << " " << unload.productType << " " << unload.count;
	return out;
}

std::ostream& operator<<(std::ostream& out, const DeliverCommand& deliver) {
	out << deliver.droneId << " D " << deliver.orderId << " " << deliver.productType << " " << deliver.count;
	return out;
}

std::ostream& operator<<(std::ostream& out, const WaitCommand& wait) {
	out << wait.droneId << " W " << wait.sleepTurns;
	return out;
}
