#pragma once

class TTNeighborsDiscovery {
public:
    TTNeighborsDiscovery() = default;
    virtual ~TTNeighborsDiscovery();
    TTNeighborsDiscovery(const TTNeighborsDiscovery&) = delete;
    TTNeighborsDiscovery(TTNeighborsDiscovery&&) = delete;
    TTNeighborsDiscovery operator=(const TTNeighborsDiscovery&) = delete;
    TTNeighborsDiscovery operator=(TTNeighborsDiscovery&&) = delete;
private:
};
