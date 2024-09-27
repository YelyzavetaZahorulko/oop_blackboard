#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>
#include <string>


// Define the size of the board
const int BOARD_WIDTH = 80;
const int BOARD_HEIGHT = 25;

class Shape {
protected:
    int x, y;
public:
    Shape(int x, int y) : x(x), y(y) {}
    virtual ~Shape() = default;

    virtual void draw(std::vector<std::vector<char>>& grid) const = 0;

    virtual std::tuple<std::string, int, int, int, int, bool> getParameters() const = 0;
};


// Struct to define the board
struct Board {
    std::vector<std::vector<char>> grid;
    std::vector<std::tuple<std::string, int, int, int, int, bool>> shapesParams; // Store shape parameters

    Board() : grid(BOARD_HEIGHT, std::vector<char>(BOARD_WIDTH, ' ')) {}

    void print() {
        for (auto& row : grid) {
            for (char c : row) {
                std::cout << c;
            }
            std::cout << "\n";
        }
    }

    bool addShape(const std::shared_ptr<Shape>& shape) {
        auto shapeParams = shape->getParameters();

        for (const auto& params : shapesParams) {
            if (params == shapeParams) {
                std::cout << "Shape already exists.\n";
                return false;
            }
        }

        // Draw the shape
        shape->draw(grid);
        shapesParams.push_back(shapeParams);
        return true;
    }
};

class Triangle: public Shape {
    int height;
public:
    Triangle(int x, int y, int height) : Shape(x, y), height(height) {}


    void draw(std::vector<std::vector<char>>& grid) const override {
        if (height <= 0) return; // Ensure the triangle height is positive and sensible
        for (int i = 0; i < height; ++i) {
            int leftMost = x - i; // Calculate the starting position
            int rightMost = x + i; // Calculate the ending position
            int posY = y + i; // Calculate the vertical position
            // Draw only the edges/border of the triangle
            if (posY < BOARD_HEIGHT) {
                if (leftMost >= 0 && leftMost < BOARD_WIDTH) // Check bounds for left most position
                    grid[posY][leftMost] = '*';
                if (rightMost >= 0 && rightMost < BOARD_WIDTH && leftMost != rightMost)
                    // Check bounds for right most position
                        grid[posY][rightMost] = '*';
            }
        }
        // Draw the base of the triangle separately
        for (int j = 0; j < 2 * height - 1; ++j) {
            int baseX = x - height + 1 + j;
            int baseY = y + height - 1;
            if (baseX >= 0 && baseX < BOARD_WIDTH && baseY < BOARD_HEIGHT) // Check bounds for each position on the base
                grid[baseY][baseX] = '*';
        }
    }

    std::tuple<std::string, int, int, int, int, bool> getParameters() const override {
        return std::make_tuple("Triangle", x, y, height, 0, false);
    }
};

class CommandLine {
    Board& board;

public:
    CommandLine(Board& b) : board(b) {}

    // Parse and execute a command
    void executeCommand(const std::string& command) {
        std::istringstream ss(command);
        std::string action, shapeType;
        int x, y, param1, param2 = 0;
        bool isVertical = false;

        ss >> action >> shapeType;

        if (action == "draw") {
            if (shapeType == "triangle") {
                ss >> x >> y >> param1;  // x, y, height
                std::shared_ptr<Shape> triangle = std::make_shared<Triangle>(x, y, param1);
                board.addShape(triangle);
            } else {
                std::cout << "Unknown shape type.\n";
            }
        } else {
            std::cout << "Unknown command.\n";
        }
    }
};

int main() {
    Board board;
    CommandLine cli(board);


    std::string command;
    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, command);

        if (command == "exit") break;

        cli.executeCommand(command);
        board.print();
    }
    return 0;
}
