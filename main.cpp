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
private:
    std::vector<std::vector<char>> grid;
    std::vector<std::tuple<int, std::string, int, int, int, int, bool>> shapesParams; // Store shape parameters
    std::vector<std::shared_ptr<Shape>> shapes;
    int currentShapeId = 1;
public:
    Board() : grid(BOARD_HEIGHT, std::vector<char>(BOARD_WIDTH, ' ')) {}

    void print() {
        std::cout << std::string(BOARD_WIDTH + 2, '-') << std::endl;
        for (auto& row : grid) {
            std::cout << "|";  // Left border
            for (char c : row) {
                std::cout << c;
            }
            std::cout << "|";  // Right border
            std::cout << "\n";
        }
        std::cout << std::string(BOARD_WIDTH + 2, '-') << std::endl;
    }

    bool addShape(const std::shared_ptr<Shape>& shape) {
        auto shapeParams = shape->getParameters();

        for (const auto& params : shapesParams) {
            if (std::get<1>(params) == std::get<0>(shapeParams) &&  // Shape type
                std::get<2>(params) == std::get<1>(shapeParams) &&  // x
                std::get<3>(params) == std::get<2>(shapeParams) &&  // y
                std::get<4>(params) == std::get<3>(shapeParams) &&  // param1
                std::get<5>(params) == std::get<4>(shapeParams)) {  // param2
                std::cout << "Shape with the same parameters already exists.\n";
                return false;
                }
        }

        shapesParams.push_back(std::make_tuple(currentShapeId++, std::get<0>(shapeParams), std::get<1>(shapeParams), std::get<2>(shapeParams), std::get<3>(shapeParams), std::get<4>(shapeParams), std::get<5>(shapeParams)));
        shapes.push_back(shape);
        return true;
    }

    void clear() {
        shapes.clear();
        for (auto& row : grid) {
            std::fill(row.begin(), row.end(), ' '); // Fill each row with empty spaces
        }
    }

    void draw() {
        for (const auto& shape : shapes) {
            shape->draw(grid);
        }
        print();
    }

    void showShapesList() {
        for (const auto& params : shapesParams) {
            int id = std::get<0>(params);
            std::string shapeType = std::get<1>(params);
            int x = std::get<2>(params);
            int y = std::get<3>(params);
            int param1 = std::get<4>(params);
            int param2 = std::get<5>(params);
            // bool isVertical = std::get<6>(params);

            std::cout << "Shape ID: " << id << ", Type: " << shapeType << ", X: " << x << ", Y: " << y;

            if (shapeType == "Triangle") {
                std::cout << ", Height: " << param1 << "\n";
            } else if (shapeType == "Circle") {
                std::cout << ", Radius: " << param1 << "\n";
            } else if (shapeType == "Rectangle") {
                std::cout << ", Width: " << param1 << ", Height: " << param2 << "\n";
            }
        }
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

class Line : public Shape {
private:
    int x1, y1, x2, y2;

public:
    Line(int startX, int startY, int endX, int endY) : Shape(startX, startY), x1(startX), y1(startY), x2(endX), y2(endY) {}

    void draw(std::vector<std::vector<char>>& grid) const override {
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1; // Step in x
        int sy = (y1 < y2) ? 1 : -1; // Step in y 
        int err = dx - dy;

        int x = x1;
        int y = y1;

        while (true) {
            // Ensure the point is within grid bounds
            if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
                grid[y][x] = '*';
            }

            // Check if we reached the endpoint
            if (x == x2 && y == y2) break;

            int e2 = 2 * err;

            // Move horizontally or vertically based on error margin
            if (e2 > -dy) {
                err -= dy;
                x += sx; // Move in x direction
            }
            if (e2 < dx) {
                err += dx;
                y += sy; // Move in y direction
            }
        }
    }

    std::tuple<std::string, int, int, int, int, bool> getParameters() const override {
        return std::make_tuple("Line", x1, y1, x2, y2, false);
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

        if (action == "add") {
            if (shapeType == "triangle") {
                ss >> x >> y >> param1;  // x, y, height
                std::shared_ptr<Shape> triangle = std::make_shared<Triangle>(x, y, param1);
                board.addShape(triangle);
                std::cout << "Triangle is succesfully added \n";
            } else if (shapeType == "circle") {
                ss >> x >> y >> param1;  // x, y, radius
                std::shared_ptr<Shape> circle = std::make_shared<Circle>(x, y, param1);
                board.addShape(circle);
                std::cout << "Circle is succesfully added \n";
            } else if (shapeType == "rectangle") {
                ss >> x >> y >> param1 >> param2;  // x, y, height, weight
                std::shared_ptr<Shape> rectangle = std::make_shared<Rectangle>(x, y, param1, param2);
                board.addShape(rectangle);
                std::cout << "Rectangle is succesfully added \n";
            } else if (shapeType == "line") {
                ss >> x >> y >> param1 >> param2;  // x1, y1, x2, y2
                std::shared_ptr<Shape> line = std::make_shared<Line>(x, y, param1, param2);
                board.addShape(line);
                std::cout << "Line is succesfully added \n";
            }
            else {
                std::cout << "Unknown shape type \n";
            }
        } else if (action == "draw"){
            board.draw();
        } else if (action == "clear"){
            board.clear();
            std::cout << "Board is succesfully cleared \n";
        } else if (action == "list") {
            board.showShapesList();
            std::cout << "\n";
        }
        else {
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
    }
    return 0;
}
