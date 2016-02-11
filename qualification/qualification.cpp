#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

#include <cassert>
#include <cmath>

typedef size_t OrderId;
typedef size_t DroneId;
typedef size_t ProductId;
typedef size_t WarehouseId;

const size_t INVALID_ID = (size_t) -1;

class Point {
public:
    size_t x;
    size_t y;

    explicit Point(size_t x, size_t y): x(x), y(y) {}

    size_t distance(const Point& other) {
        return static_cast<size_t>(ceil(sqrt(pow(x - other.x, 2) + pow(y - other.y, 2))));
    }
};

std::ostream& operator<<(std::ostream& out, const Point& point) {
    out << "(" << point.x << ", " << point.y << ")";
    return out;
}

class Warehouse {
public:
    WarehouseId id;
    Point position;
    std::unordered_map<ProductId, size_t> m_products_available;

    void add_product(ProductId id, size_t available) {
        m_products_available[id] = available;
    }

    bool has(ProductId id, size_t amount) {
        return m_products_available[id] >= amount;
    }

    bool has(ProductId id) {
        return has(id, 1);
    }

    void take(ProductId id, size_t amount) {
        assert(has(id, amount));
        m_products_available[id] -= amount;
    }

    void take(ProductId id) {
        take(id, 1);
    }

    explicit Warehouse(WarehouseId id, size_t x, size_t y): position(x, y) {}
};

class Product {
public:
    ProductId id;
    size_t weight;

    explicit Product(ProductId id, size_t weight): id(id), weight(weight) {}
};

class Order {
public:
    OrderId id;
    Point destination;
    std::vector<ProductId> m_products;
    std::vector<bool> m_delivered;

    explicit Order(OrderId id, size_t x, size_t y): id(id), destination(x, y) {}

    void add_product(ProductId id) {
        m_products.push_back(id);
        m_delivered.push_back(false);
    }

    ProductId next_undelivered_product() {
        size_t size = m_delivered.size();
        for (size_t i = 0; i < size; ++i) {
            if (!m_delivered[i]) {
                return m_products[i];
            }
        }
        return INVALID;
    }
};

class Drone {
public:
    DroneId id;
    Point position;
    std::vector<ProductId> current_products;

    explicit Drone(DroneId id, size_t x, size_t y): id(id), position(x, y) {}
};

class Simulation {
public:
    size_t m_current_turn;
    size_t m_width;
    size_t m_height;
    size_t m_turns_deadline;
    size_t m_drone_max_load;

    std::vector<Drone> m_drones;

    std::vector<Product> m_products;

    std::vector<Warehouse> m_warehouses;

    std::vector<Order> m_orders;
    explicit Simulation(std::ifstream& in);

    DroneId nearest_unbusy_drone(const Point& point) {
    }
};

Simulation::Simulation(std::ifstream& in) : m_current_turn(0) {
    std::string line;
    assert(std::getline(in, line));

    size_t drone_count;
    {
        std::istringstream iss(line); // Overkill wut...
        iss >> m_height;
        iss >> m_width;

        iss >> drone_count;
        m_drones.reserve(drone_count);

        iss >> m_turns_deadline;
        iss >> m_drone_max_load;
    }

    assert(std::getline(in, line));

    size_t product_count = std::stoul(line);
    assert(product_count < 10000);

    m_products.reserve(product_count);

    assert(std::getline(in, line));

    {
        std::istringstream iss(line);

        for (size_t i = 0; i < product_count; i++) {
            size_t weight;
            iss >> weight;

            assert(weight < m_drone_max_load);
            m_products.push_back(Product(i, weight));
        }
    }

    assert(std::getline(in, line));
    size_t warehouse_count = std::stoul(line);
    assert(warehouse_count < 10000);

    m_warehouses.reserve(warehouse_count);

    for (size_t i = 0; i < warehouse_count; i++) {
        assert(std::getline(in, line));

        size_t x, y;
        {
            std::istringstream iss(line);
            iss >> y;
            iss >> x;
        }

        Warehouse this_warehouse(i, x, y);

        assert(x < m_width);
        assert(y < m_height);

        assert(std::getline(in, line));
        {
            std::istringstream iss(line);
            for (auto& product: m_products) {
                size_t this_product_count;
                iss >> this_product_count;
                this_warehouse.add_product(product.id, this_product_count);
            }
        }

        m_warehouses.push_back(this_warehouse);
    }

    assert(std::getline(in, line));
    size_t order_count = std::stoul(line);

    assert(order_count < 10000);
    for (size_t i = 0; i < order_count; i++) {
        assert(std::getline(in, line));
        size_t x, y;
        {
            std::istringstream iss(line);
            iss >> x;
            iss >> y;
        }

        Order this_order(i, x, y);

        assert(std::getline(in, line));
        size_t product_count = std::stoul(line);

        assert(std::getline(in, line));
        std::istringstream iss(line);
        for (size_t i = 0; i < product_count; i++) {
            ProductId id;
            iss >> id;
            assert(id < m_products.size());
            this_order.add_product(id);
        }

        m_orders.push_back(this_order);
    }

    for (size_t i = 0; i < drone_count; i++) {
        m_drones.push_back(Drone(i, m_warehouses[0].position.x, m_warehouses[0].position.y));
    }

    std::cout << "(" << m_width << ", " << m_height << ")" << std::endl;
    std::cout << m_drones.size() << " drones" << std::endl;
    std::cout << m_warehouses.size() << " warehouses" << std::endl;
    std::cout << m_products.size() << " products" << std::endl;
    std::cout << m_orders.size() << " orders" << std::endl;
}

int main(int argc, char** argv) {
    if (argc <= 2) {
        std::cerr << "Usage: " << argv[0] << " <in> <out>" << std::endl;
        return 1;
    }

    std::ifstream in(argv[1]);
    assert(in);

    Simulation simulation(in);

    auto commands = simulation.simulate();

    for (size_t turn = 0; i < simulation.m_turn_deadline; ++turn) {
        for (auto& order: simulation.m_order) {
            ProductId next_product_to_deliver = order.next_undelivered_product();
            if (next_product_to_deliver != INVALID)
                continue; // Next order

            WarehouseId warehouse = simulation.nearest_warehouse_with_product(order.destination, next_product_to_deliver);

            DroneId drone_id = simulation.nearest_unbusy_drone(order.destination);
        }

        while (!simulation.m_orders.empty()) {
            const Order& order = simulation.m_orders[0];
            const next_undelivered_product = product

            auto& next_unused_drone = simulation.
            next_order
        }
    }
}
