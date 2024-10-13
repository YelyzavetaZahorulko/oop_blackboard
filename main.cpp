#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>
#include <string>
#include <fstream>

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

    virtual bool containsPoint(int px, int py) const = 0;

    void setID(int id) { shapeID = id; }
    int getID() const { return shapeID; }

private:
    int shapeID;
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

    bool containsPoint(int px, int py) const override {
        // Check if the point is on the left or right edge
        for (int i = 0; i < height; ++i) {
            int leftMost = x - i;
            int rightMost = x + i;
            int posY = y + i;
            if (posY == py) {
                if (px == leftMost || px == rightMost) {
                    return true;
                }
            }
        }
        // Check the base
        int baseY = y + height - 1;
        if (py == baseY) {
            if (px >= (x - height + 1) && px <= (x + height - 1)) {
                return true;
            }
        }
        return false;
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

    bool containsPoint(int px, int py) const override {
        int dx = px - x;
        int dy = py - y;
        int distSquared = dx * dx + dy * dy;
        int r2 = radius * radius;
        return (distSquared >= (r2 - radius) && distSquared <= (r2 + radius));
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

    bool containsPoint(int px, int py) const override {
        // Check if the point is on the top or bottom edge
        if (py == y || py == y + height - 1) {
            if (px >= x && px < x + width) {
                return true;
            }
        }
        // Check the left and right edges
        if ((px == x || px == x + width - 1) && py >= y && py < y + height) {
            return true;
        }
        return false;
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

    bool containsPoint(int px, int py) const override {
        // Implement Bresenham's line algorithm to check if the point is on the line
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        int x = x1;
        int y = y1;

        while (true) {
            if (x == px && y == py) {
                return true;
            }

            if (x == x2 && y == y2) {
                break;
            }

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }

        return false;
    }

    std::tuple<std::string, int, int, int, int, bool> getParameters() const override {
        return std::make_tuple("Line", x1, y1, x2, y2, false);
    }
};

struct Board {
private:
    std::vector<std::vector<char>> grid;
    std::vector<std::tuple<int, std::string, int, int, int, int, bool>> shapesParams; // Store shape parameters
    std::vector<std::shared_ptr<Shape>> shapes;
    int currentShapeID = 1;
    int selectedShapeID = -1;
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

        shape->setID(currentShapeID);
        shapesParams.push_back(std::make_tuple(currentShapeID++, std::get<0>(shapeParams), std::get<1>(shapeParams), std::get<2>(shapeParams), std::get<3>(shapeParams), std::get<4>(shapeParams), std::get<5>(shapeParams)));
        shapes.push_back(shape);
        currentShapeID++;
        return true;
    }

    void clear() {
        shapes.clear();
        shapesParams.clear();
        selectedShapeID = -1;
        for (auto& row : grid) {
            std::fill(row.begin(), row.end(), ' '); // Fill each row with empty spaces
        }
    }

    void undoClear() {
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
            } else if (shapeType == "Line") {
                std::cout << ", X2: " << param1 << ", Y2: " << param2 << "\n";
            }
        }
    }

    void availableShapes() const {
        std::cout << "Triangle: coordinates, height\n";
        std::cout << "Circle: coordinates, radius\n";
        std::cout << "Rectangle: coordinates, height, width\n";
        std::cout << "Line: start coordinates, end coordinates\n";
    }

    void undo() {
        if (!shapes.empty()) {
            shapes.pop_back();  // Remove the last added shape
            shapesParams.pop_back();  // Remove its parameters
            undoClear();
            std::cout << "Last shape removed from the board.\n";
            draw();  // Redraw the board with remaining shapes
        } else {
            std::cout << "No shapes to remove.\n";
        }
    }

    void save(const std::string& filename) {
        std::ofstream outFile(filename, std::ios::out);
        if (!outFile) {
            std::cout << "Error opening file for saving.\n";
            return;
        }

        // Save each shape's parameters
        for (const auto& shape : shapes) {
            auto params = shape->getParameters();
            std::string type = std::get<0>(params);
            int x = std::get<1>(params);
            int y = std::get<2>(params);
            int param1 = std::get<3>(params);
            int param2 = std::get<4>(params);

            outFile << type << " " << x << " " << y << " " << param1 << " " << param2 << "\n";
        }

        outFile.close();
        std::cout << "Blackboard saved to " << filename << ".\n";
    }

    void load(const std::string& filename) {
        // Open file in read mode
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            std::cout << "File not found. Creating a new file: " << filename << ".\n";
            std::ofstream outFile(filename);  // Create new file
            outFile.close();
            return;
        }

        // Clear the current board and shapes
        clear();

        std::string type;
        int x, y, param1, param2;

        // Load each shape from the file and add to the board
        while (inFile >> type) {
            std::cout << "Loaded shape: " << type << " at (" << x << ", " << y << ") with params: " << param1 << " " << param2 << "\n";
            if (type == "Triangle") {
                inFile >> x >> y >> param1;
                std::shared_ptr<Shape> triangle = std::make_shared<Triangle>(x, y, param1);
                addShape(triangle);
            } else if (type == "Circle") {
                inFile >> x >> y >> param1;
                std::shared_ptr<Shape> circle = std::make_shared<Circle>(x, y, param1);
                addShape(circle);
            } else if (type == "Rectangle") {
                inFile >> x >> y >> param1 >> param2;
                std::shared_ptr<Shape> rectangle = std::make_shared<Rectangle>(x, y, param1, param2);
                addShape(rectangle);
            } else if (type == "Line") {
                inFile >> x >> y >> param1 >> param2;
                std::shared_ptr<Shape> line = std::make_shared<Line>(x, y, param1, param2);
                addShape(line);
            }
        }

        inFile.close();
        std::cout << "Blackboard loaded from " << filename << ".\n";
    }

    void select() {
        std::cout << "Select by ID or coordinates? (id/coord): ";
        std::string choice;
        std::cin >> choice;

        if (choice == "id") {
            int id;
            std::cout << "Enter Shape ID: ";
            std::cin >> id;
            bool found = false;
            for (const auto& params : shapesParams) {
                if (std::get<0>(params) == id) {
                    selectedShapeID = id;
                    printShapeInfo(params);
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << "Shape with ID " << id << " not found.\n";
            }
        }
        else if (choice == "coord") {
            int px, py;
            std::cout << "Enter X coordinate: ";
            std::cin >> px;
            std::cout << "Enter Y coordinate: ";
            std::cin >> py;

            bool found = false;
            for (int i = shapes.size() - 1; i >= 0; --i) {
                if (shapes[i]->containsPoint(px, py)) {
                    selectedShapeID = std::get<0>(shapesParams[i]);
                    printShapeInfo(shapesParams[i]);
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << "No shape occupies the point (" << px << ", " << py << ").\n";
            }
        }
        else {
            std::cout << "Invalid selection option.\n";
        }
    }

    // for select method
    void printShapeInfo(const std::tuple<int, std::string, int, int, int, int, bool>& params) const {
        int id = std::get<0>(params);
        std::string shapeType = std::get<1>(params);
        int x = std::get<2>(params);
        int y = std::get<3>(params);
        int param1 = std::get<4>(params);
        int param2 = std::get<5>(params);

        std::cout << "Selected Shape ID: " << id << ", Type: " << shapeType << ", X: " << x << ", Y: " << y;

        if (shapeType == "Triangle") {
            std::cout << ", Height: " << param1 << "\n";
        }
        else if (shapeType == "Circle") {
            std::cout << ", Radius: " << param1 << "\n";
        }
        else if (shapeType == "Rectangle") {
            std::cout << ", Width: " << param1 << ", Height: " << param2 << "\n";
        }
        else if (shapeType == "Line") {
            std::cout << ", End X: " << param1 << ", End Y: " << param2 << "\n";
        }
    }

    void removeShape() {
        if (selectedShapeID == -1) {
            std::cout << "No shape selected to remove.\n";
            return;
        }

        bool found = false;
        for (int i = 0; i < shapesParams.size(); ++i) {
            if (std::get<0>(shapesParams[i]) == selectedShapeID) {
                shapesParams.erase(shapesParams.begin() + i);
                shapes.erase(shapes.begin() + i); // Remove the shape from the shapes vector
                std::cout << "Shape with ID " << selectedShapeID << " removed successfully.\n";
                selectedShapeID = -1; // Reset the selected shape ID
                found = true;
                break;
            }
        }

        if (!found) {
            std::cout << "Shape with ID " << selectedShapeID << " not found.\n";
        }
    }

};

class CommandLine {
    Board& board;

public:
    CommandLine(Board& b) : board(b) {}

    // Parse and execute a command
    void executeCommand(const std::string& command) {
        std::istringstream ss(command);
        std::string action, shapeType, filename;
        int x, y, param1, param2 = 0;
        bool isVertical = false;

        ss >> action ;

        if (action == "save") {
            ss >> filename;
            board.save(filename);
            return;
        } else if (action == "load") {
            ss >> filename;
            board.load(filename);
            return;
        }

        if (action == "add") {
            ss >> shapeType;
            if (shapeType == "triangle" ) {
                if (ss >> x >> y >> param1) {
                    if (x >= 0 && x <= BOARD_WIDTH && y >= 0 && y <= BOARD_HEIGHT) {
                        // x, y, height
                        std::shared_ptr<Shape> triangle = std::make_shared<Triangle>(x, y, param1);
                        board.addShape(triangle);
                        std::cout << "Triangle is succesfully added \n";
                    }
                    else {
                        std::cout << "Error: Triangle's position is out of the board boundaries.\n";
                    }
                }
                else {
                    std::cout << "Error: Missing parameters for triangle. Expected x, y, height. Or figure out of the board\n";
                }
            } else if (shapeType == "circle") {
                if (ss >> x >> y >> param1 ) {
                    if (x - param1 >= 0 || x + param1 <= BOARD_WIDTH || y - param1 >= 0 || y + param1 <= BOARD_HEIGHT) {
                    // x, y, radius
                    std::shared_ptr<Shape> circle = std::make_shared<Circle>(x, y, param1);
                    board.addShape(circle);
                    std::cout << "Circle is succesfully added \n";
                }
                    else {
                        std::cout << "Error: Circle's position or radius is out of the board boundaries.\n";
                    }
                }
                else {
                    std::cout << "Error: Missing parameters for circle. Expected x, y, radius.\n";
                }
            } else if (shapeType == "rectangle") {
                if (ss >> x >> y >> param1 >> param2) {
                    if (x >= 0 && x + param1 <= BOARD_WIDTH && y >= 0 && y + param2 <= BOARD_HEIGHT) {
                        // x, y, height, weight
                        std::shared_ptr<Shape> rectangle = std::make_shared<Rectangle>(x, y, param1, param2);
                        board.addShape(rectangle);
                        std::cout << "Rectangle is succesfully added \n";
                    }
                    else {
                        std::cout << "Error: Line's position or size is out of the board boundaries.\n";
                    }
                }
                else {
                    std::cout << "Error: Missing parameters for rectangle. Expected x, y, height, weight.\n";
                }
            } else if (shapeType == "line") {
                if (ss >> x >> y >> param1 >> param2) {
                    // x1, y1, x2, y2
                    if((x >= 0 && x <= BOARD_WIDTH && y >= 0 && y <= BOARD_HEIGHT) || (param1 >= 0 && param1 <= BOARD_WIDTH && param2 >= 0 && param2 <= BOARD_HEIGHT)) {
                        std::shared_ptr<Shape> line = std::make_shared<Line>(x, y, param1, param2);
                        board.addShape(line);
                        std::cout << "Line is succesfully added \n";
                    }
                    else {
                        std::cout << "Error: Line's start or end position is out of the board boundaries.\n";
                    }

                }
                else {
                    std::cout << "Error: Missing parameters for line. Expected x1, y1, x2, y2.\n";
                }
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
        } else if (action == "shapes") {
            board.availableShapes();
            std::cout << "\n";
        } else if (action == "undo") {
            board.undo();
            std::cout << "\n";
        } else if (action == "select") {
            board.select();
            std::cout << "\n";
        } else if (action == "remove") {
            board.removeShape();
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
