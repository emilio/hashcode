#include "commands.h"

LoadCommand::LoadCommand(int droneId, int warehouseId, int productType, int count) : droneId(droneId), warehouseId(warehouseId), productType(productType), count(count) {
}

UnloadCommand::UnloadCommand(int droneId, int warehouseId, int productType, int count) : droneId(droneId), warehouseId(warehouseId), productType(productType), count(count) {
}

DeliverCommand::DeliverCommand(int droneId, int orderId, int productType, int count) : droneId(droneId), orderId(orderId), productType(productType), count(count) {
}

WaitCommand::WaitCommand(int droneId, int sleepTurns) : droneId(droneId), sleepTurns(sleepTurns){

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
