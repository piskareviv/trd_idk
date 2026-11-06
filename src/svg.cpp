#include <stdint.h>

#include <fstream>
#include <sstream>

template <typename T>
struct Vector {
    T x, y;

    Vector() : x(0), y(0) { ; }

    Vector(T x, T y) : x(x), y(y) { ; }

    // template <typename T2>
    // operator Vector<T2>() const {
    //     return Vector<T2>(x, y);
    // }

    Vector operator+(const Vector& other) const {
        return Vector(x + other.x, y + other.y);
    }

    Vector operator-() const {
        return Vector(-x, -y);
    }

    Vector operator-(const Vector& other) const {
        return Vector(x - other.x, y - other.y);
    }

    T operator*(const Vector& other) const {
        return x * other.x + y * other.y;
    }

    T operator%(const Vector& other) const {
        return x * other.y - y * other.x;
    }

    Vector<double> operator*(const double& val) const {
        return Vector(x * val, y * val);
    }

    bool operator==(const Vector& other) const {
        return x == other.x && y == other.y;
    }
};

struct Svg {
    std::stringstream sout;

    static constexpr double scale = 500;
    static constexpr double shift = 50;

    Svg() {
        clear();
    }

    void clear() {
        sout = std::stringstream();
        sout.precision(5);
        sout << std::fixed;
        sout << R"meow(<svg width=\"1000px\" height=\"1000px\" style=\"background-color:lightgreen\"
        xmlns=\"http://www.w3.org/2000/svg\">\n)meow";
    }

    void print() {
        std::string s = sout.str();
        s += "</svg>\n";

        std::ofstream fout("meow.svg");
        fout << s << "\n";
        fout.flush();
        fout.close();
    }

    void line(Vector<double> pt1, Vector<double> pt2, std::string color, double width = 1) {
        sout << "<line ";
        sout << "x1=\"" << (float)(pt1.x * scale + shift) << "\" ";
        sout << "y1=\"" << (float)((scale - pt1.y * scale) + shift) << "\" ";
        sout << "x2=\"" << (float)(pt2.x * scale + shift) << "\" ";
        sout << "y2=\"" << (float)((scale - pt2.y * scale) + shift) << "\" ";
        sout << " stroke=\"" << color << "\"";
        sout << " stroke-width=\"" << float(width) << "\"";
        sout << "/>\n";
    }

    void circle(Vector<double> pt, double r, std::string color, double width = 1) {
        sout << "<circle ";
        sout << "cx=\"" << float(pt.x * scale + shift) << "\" ";
        sout << "cy=\"" << float((scale - pt.y * scale) + shift) << "\" ";
        sout << "r=\"" << float(r) << "\" ";
        sout << " stroke=\"" << color << "\"";
        sout << " stroke-width=\"" << float(width) << "\"";
        sout << "/>\n";
    };
};
