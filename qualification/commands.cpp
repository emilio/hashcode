#include "commands.h"

Load::Load(int droneId, int warehouseId, int productType, int count) : droneId(droneId), warehouseId(warehouseId), productType(productType), count(count) {
}

Unload::Unload(int droneId, int warehouseId, int productType, int count) : droneId(droneId), warehouseId(warehouseId), productType(productType), count(count) {
}

Deliver::Deliver(int droneId, int orderId, int productType, int count) : droneId(droneId), orderId(orderId), productType(productType), count(count) {
}

Wait::Wait(int droneId, int sleepTurns) : droneId(droneId), sleepTurns(sleepTurns){
	
}

std::ostream& operator<<(std::ostream& out, const Load& load) {
	out << load.droneId << " L " << load.warehouseId << " " << load.productType << " " << load.count;
	return out;
}

std::ostream& operator<<(std::ostream& out, const Unload& unload) {
	out << unload.droneId << " U " << unload.warehouseId << " " << unload.productType << " " << unload.count;
	return out;
}

std::ostream& operator<<(std::ostream& out, const Deliver& deliver) {
	out << deliver.droneId << " D " << deliver.orderId << " " << deliver.productType << " " << deliver.count;
	return out;
}

std::ostream& operator<<(std::ostream& out, const Wait& wait) {
	out << wait.droneId << " W " << wait.sleepTurns;
	return out;
}
