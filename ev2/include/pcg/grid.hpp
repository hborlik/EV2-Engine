/**
 * @file grid.hpp
 * @brief 
 * @date 2023-03-03
 * 
 */
#ifndef EV2_GRID_HPP
#define EV2_GRID_HPP

#include <stdexcept>

namespace pcg {

template<typename T>
class Grid {
public:
    Grid(int width, int height): width{ width }, height{ height } {
        if (width < 0 || height < 0)
            throw std::logic_error("Grid::Grid invalid size argument");
        grid = new T[width * height];
    }

    ~Grid() {
        delete[] grid;
    }

    const T& at(int x, int y) const {
        if (x < 0 || x > width || y < 0 || y > height)
            throw std::out_of_range("Grid::at (x, y) out of range");
        return grid[x * width + y];
    }

    T& at(int x, int y) {
        if (x < 0 || x > width || y < 0 || y > height)
            throw std::out_of_range("Grid::at (x, y) out of range");
        return grid[x * width + y];
    }

    const int width, height;
private:
    T grid[];
};

}

#endif // EV2_GRID_HPP