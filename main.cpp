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

class Circle : public Shape {
    int radius;

public:
    Circle(int x, int y, int radius) : Shape(x, y), radius(radius) {}

    void draw(std::vector<std::vector<char>>& grid) const override {
        if (radius <= 0) return;

        int r2 = radius * radius;  // Precompute radius squared to compare distances
        for (int i = 0; i < BOARD_HEIGHT; ++i) {
            for (int j = 0; j < BOARD_WIDTH; ++j) {
                // Calculate the squared distance from the point (i, j) to the center (x, y)
                int dx = j - x;
                int dy = i - y;
                int distSquared = dx * dx + dy * dy;

                // If the squared distance is close to the radius squared, draw the border
                if (distSquared >= (r2 - radius) && distSquared <= (r2 + radius)) {
                    grid[i][j] = '*';
                }
            }
        }
    }

    // Return the shape's parameters as a tuple
    std::tuple<std::string, int, int, int, int, bool> getParameters() const override {
        return std::make_tuple("Circle", x, y, radius, 0, false);
    }
};

class Rectangle : public Shape {
    int width;
    int height;

public:
    Rectangle(int x, int y, int width, int height) : Shape(x, y), width(width), height(height) {}

    void draw(std::vector<std::vector<char>>& grid) const override {
        if (width <= 0 || height <= 0) return;

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                int gridX = x + j; // Calculate grid x position
                int gridY = y + i; // Calculate grid y position

                // Draw the top and bottom borders
                if (i == 0 || i == height - 1) {
                    if (gridY < BOARD_HEIGHT && gridX < BOARD_WIDTH) {
                        grid[gridY][gridX] = '*'; // Draw top and bottom edges
                    }
                }
                // Draw the left and right borders
                else if (j == 0 || j == width - 1) {
                    if (gridY < BOARD_HEIGHT && gridX < BOARD_WIDTH) {
                        grid[gridY][gridX] = '*'; // Draw left and right edges
                    }
                }
            }
        }
    }

    // Method to return the shape's parameters
    std::tuple<std::string, int, int, int, int, bool> getParameters() const override {
        return std::make_tuple("Rectangle", x, y, width, height, false);
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
            } else if (shapeType == "circle") {
                ss >> x >> y >> param1;  // x, y, radius
                std::shared_ptr<Shape> circle = std::make_shared<Circle>(x, y, param1);
                board.addShape(circle);
            } else if (shapeType == "rectangle") {
                ss >> x >> y >> param1 >> param2;  // x, y, height, weight
                std::shared_ptr<Shape> rectangle = std::make_shared<Rectangle>(x, y, param1, param2);
                board.addShape(rectangle);
            }
            else {
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
