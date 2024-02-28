#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // For std::min_element and std::max_element
#include <thread>
#include <sstream>
#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include "math/BoundingBox.h"

namespace statistic
{

    struct Point
    {
        double value;
        double occurrences;
    };
    inline math::Bounds2D<double> getRange(const std::vector<Point>& points)
    {
        math::Bounds2D<double> bounds;
        for (const Point point : points)
            bounds.Encapsulate(glm::dvec2(point.value, point.occurrences));


        return bounds;
    }

    inline void svgLine(std::ostream& svg, double x0, double y0, double x1, double y1, std::string_view color, double width) {
        svg << "<line x1=\"" << x0 << "\" y1=\"" << y0
            << "\" x2=\"" << x1 << "\" y2=\"" << y1
            << "\" style=\"stroke:" << color << ";stroke-width:" << width << "\" />\n";
    }
    inline void svgCircle(std::ostream& svg, double cx, double cy, double r, std::string_view strokeColor, double strokeWidth, std::string_view fillColor) {
        svg << "<circle cx=\"" << cx << "\" cy=\"" << cy
            << "\" r=\"" << r
            << "\" stroke=\"" << strokeColor << "\" stroke-width=\"" << strokeWidth
            << "\" fill=\"" << fillColor << "\" />\n";
    }

    inline void svgText(std::ostream& svg, double x, double y, double fontSize, std::string_view content, std::string_view color = "#000000", std::string_view fontFamily = "Arial") {
        svg << "<text x=\"" << x << "\" y=\"" << y
            << "\" fill=\"" << color << "\" font-family=\"" << fontFamily
            << "\" font-size=\"" << fontSize << "\">"
            << content << "</text>\n";
    }

    inline double inverse_lerp(double x0, double x1, double v) {
        // Avoid division by zero if x0 == x1
        if (x0 == x1) return 0.0;
        return (v - x0) / (x1 - x0);
    }

    inline std::string generateExcel(const std::vector<double>& values)
    {
        std::stringstream ss;

        for (const double point : values)
        {
            ss << static_cast<size_t>(point) << ";";
            ss << "\n";
        }
        return ss.str();
    }

    inline std::string generateExcel(const std::vector<Point>& points)
    {
        std::stringstream ss;

        for(auto point:points)
        {
            ss << static_cast<size_t>(point.value) << ";";
            ss << static_cast<size_t>(point.occurrences) << ";";
            ss << "\n";
        }
        return ss.str();
    }


    inline std::string generateSVG(const std::vector<Point>& points,
        const math::Bounds2D<double>& viewport, size_t originalWidth, size_t originalHeight, double base_y = 1000) {
        if (points.size() < 3) return ""; // Need at least 3 points to find a peak

        std::stringstream svg;
        size_t width = originalWidth + 100;
        size_t height = originalHeight + 100;

        if(base_y < 1)
        {
            base_y = viewport.height();
        }


        double yscale = viewport.height() / log(viewport.height());


        auto pixelSpace = [&](const Point& p) -> glm::dvec2 {
            const double x = inverse_lerp(viewport.x0, viewport.x1, p.value) * originalWidth; 

            const int sign = glm::sign(p.occurrences);
        	double y = log(abs(p.occurrences) + 1) * sign * yscale;

        	y = inverse_lerp(viewport.y0, viewport.y1, y) * originalHeight;

            return glm::dvec2(x + 50, height - y - 50);
            };

        svg << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";

        size_t lastPeak = 0;
        glm::dvec2 last = pixelSpace(points[0]);

        std::vector<glm::dvec2> pixelSpaceCoords;
        pixelSpaceCoords.reserve(points.size());

        for (size_t i = 0; i < points.size(); ++i) {

            glm::dvec2 current = pixelSpace(points[i]);
            pixelSpaceCoords.emplace_back(current);
        }

        std::vector<std::pair<size_t, double>> peaks;

        peaks.emplace_back(0, 0);
        int32_t sign = 1;

        for (size_t i = 1; i < points.size(); ++i) {

            glm::dvec2& last = pixelSpaceCoords[i - 1];
            glm::dvec2& current = pixelSpaceCoords[i];
            glm::dvec2 delta = current - last;
            double steepness = delta.y / delta.x;
            
            if(sign != static_cast<int>(glm::sign(steepness)))
            {
                peaks.back().second = sign*glm::distance(points[i-1].occurrences, points[peaks.back().first].occurrences);
                peaks.emplace_back(i-1, 0);
                sign = -sign;
            }

            svgLine(svg, last.x, last.y, current.x, current.y, "#0055FF", 2);
        }

        for (size_t i = 1; i < peaks.size(); i++) {
            auto& peak = peaks[i];
            if (peak.second) {
                glm::dvec2& current = pixelSpaceCoords[peak.first];
                double radius = sqrt(sqrt(peak.second / 5.0));
                if(radius > 2)
                {
                    svgCircle(svg, current.x, current.y, radius, "#FF0000", 1, "#FF0000");

                    double fontSize = std::max(10.0, sqrt(peak.second) * 0.5); // Ensure a minimum font size of 10
                    std::string content = "(" + std::to_string(static_cast<int>(current.x)) + "," + std::to_string(static_cast<int>(current.y)) + ")";
                    svgText(svg, current.x + radius + 2, current.y, fontSize, content, "#000000");
                }
            	
            }
        }
        
        glm::dvec2 finalPoint = pixelSpace(points.back());
        svgLine(svg, last.x, last.y, finalPoint.x, finalPoint.y, "#0055FF", 2);

        svg << "</svg>";

        return svg.str();
    }

    inline double mean(const std::vector<Point>& points) {

        size_t n = 0;
        double sum = 0;
        for(const Point p: points)
        {

            sum += p.value * p.occurrences;
            n += static_cast<size_t>(p.occurrences);
        }
        return sum/static_cast<double>(n);
    }
    inline std::vector<double> flatten(const std::vector<Point>& points, double percentage = 50) {

        std::vector<double> values;
        for (const Point p : points)
        {
            const size_t occurrences = static_cast<size_t>(p.occurrences);
            for(size_t i = 0;i< occurrences;i++)
            {
                values.emplace_back(p.value);
            }
        }
        return values;
    }
    inline double linear_sample(const std::vector<double>& values, double percentage = 50) {

        if(percentage < 0 || percentage > 100)
        {
            throw std::exception("percentage cannot be out of range of the vector");
        }
        if(values.empty())
        {
            return 0;
        }
        const size_t max = values.size() - 1;
        const double index = std::lerp(0ull, max, percentage / 100.0);
        const double left = glm::fract(index);
        const size_t pos_left = static_cast<size_t>(index);
        const size_t pos_right = static_cast<size_t>(index) + (pos_left != max);

        return values[pos_left] * left + values[pos_right]*(1.0-left);
    }
   
    inline double variance(const std::vector<Point>& points, double mean) {
        size_t n = 0;
        double variance = 0;
    	for (const Point p : points)
        {
            const double diff = (p.value-mean);
            variance += diff * diff * p.occurrences;
            n += static_cast<size_t>(p.occurrences);
        }
        return variance / static_cast<double>(n);
    }
    inline std::vector<std::pair<double, double>> take_medians(const std::vector<double>& values, std::initializer_list<double> percentages, bool median = true) {

        if(values.empty())
        {
            return {};
        }
        std::vector<std::pair<double, double>> samples;

        for(auto position: percentages)
        {
            samples.emplace_back(position, linear_sample(values, position));
        }
        if(median)
        {
            samples.emplace_back(50, linear_sample(values, 50));
        }
        for (auto it = std::rbegin(percentages); it != std::rend(percentages); ++it)
        {
            double position = 100.0 - *it;
            samples.emplace_back(position, linear_sample(values, position));
        }
        return samples;
    }
}
