// ============================================================================
// Road junctions and display. Code based on initial JS code at
// https://github.com/gniziemazity/virtual-world by Radu Mariescu-Istodor.
// See also his Youtube video https://youtu.be/3Aqe7Tv1jug?si=NN2ANPUDVZIJNs43
// "A Virtual World - JavaScript Course: Lesson 4 / 11  [Polygon Operations in
// JavaScript".
// Note: I have changed name of classes.
// ============================================================================

#include "Application.hpp"
#include <cmath>
#include <algorithm>

const float EPS = 1E-9;

//-------------------------------------------------------------------------
//! \brief Used for showing segment intersections. Only used for debug.
//-------------------------------------------------------------------------
static void drawCircle(sf::RenderTarget& target, sf::Vector2f const& p)
{
    sf::CircleShape shape(6.0f, 16);
    shape.setOrigin(shape.getRadius(), shape.getRadius());
    shape.setPosition(p);
    shape.setFillColor(sf::Color(255, 0, 255));
    shape.setOutlineThickness(2.0f);
    shape.setOutlineColor(sf::Color(255, 0, 255));
    target.draw(shape);// states);
}

//-----------------------------------------------------------------------------
//! \brief Linear interpolation between a and b. t shall be between 0 (that
//! will return a) and 1 (that will return b). t range is not checked by this
//! fucntion.
//-----------------------------------------------------------------------------
template<class T>
static T lerp(T const a, T const b, float const t)
{
    return a + (b - a) * t;
}

//-----------------------------------------------------------------------------
//! \brief Distance between two points.
//! \note: Added because missing in the SFML lib.
//-----------------------------------------------------------------------------
static float distance(sf::Vector2f const& p1, sf::Vector2f const& p2)
{
   return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                    (p1.y - p2.y) * (p1.y - p2.y));
}

//------------------------------------------------------------------------------
//! \brief Return the dot product.
//------------------------------------------------------------------------------
inline float dot(sf::Vector2f const& a, sf::Vector2f const& b)
{
   return a.x * b.x + a.y * b.y;
}

//-----------------------------------------------------------------------------
//! \brief Return the angle between 2 points.
//-----------------------------------------------------------------------------
static float orientation(sf::Vector2f const& p1, sf::Vector2f const& p2)
{
    sf::Vector2f const p = p1 - p2;
    return std::atan2(p.y, p.x);
}

//-----------------------------------------------------------------------------
//! \brief Used for computing vertices of the capsule (road shape).
//! \param[in] c center of the circle.
//! \param[in] angle angle of rotation.
//! \param[in] radius radius of the circle.
//! \return the new vertex on the circle.
//-----------------------------------------------------------------------------
static sf::Vector2f translate(sf::Vector2f const& c,
                              float const angle, float const radius)
{
    return { c.x + std::cos(angle) * radius, c.y + std::sin(angle) * radius };
}

//-----------------------------------------------------------------------------
//! \brief Compute the determinant of a 2x2 matrix:
//!   | a b |
//!   | c d |
//! which is: ad - bc.
//-----------------------------------------------------------------------------
static float determinant(float const a, float const b,
                         float const c, float const d)
{
    return a * d - b * c;
}

//-----------------------------------------------------------------------------
//! \brief bounding box check necessary for the case when the segments
//! belong to the same line, and (being a lightweight check) it allows the
//! algorithm to work faster on average on random tests.
//-----------------------------------------------------------------------------
bool operator<(sf::Vector2f const& p1, sf::Vector2f const& p2)
{
    return (p1.x < (p2.x - EPS)) ||
           ((std::abs(p1.x - p2.x) < EPS) && (p1.y < p2.y - EPS));
}

std::ostream& operator<<(std::ostream& os, sf::Vector2f const& p)
{
    return os << "(" << p.x << ", " << p.y << ")";
}

// FIXME: std::max ne compile pas
sf::Vector2f max(const sf::Vector2f& a, const sf::Vector2f& b)
{
    if (a < b)
        return b;
    return a;
}

sf::Vector2f min(const sf::Vector2f& a, const sf::Vector2f& b)
{
    if (a < b)
        return a;
    return b;
}

//-----------------------------------------------------------------------------
//! \brief Check if p is between left and right bounds of a segment.
//-----------------------------------------------------------------------------
inline bool between(float const p, float const left, float const right)
{
    return (std::min(left, right) <= (p + EPS)) &&
            (p <= (std::max(left, right) + EPS));
}

// ============================================================================
//! \brief Helper class for normalized line equation: a x + b y + c = 0
//! See https://cp-algorithms.com/geometry/lines-intersection.html
//! See https://cp-algorithms.com/geometry/segments-intersection.html
// ============================================================================
struct LineEquation
{
    //-------------------------------------------------------------------------
    //! \brief Create the normalized line equation from two points taken on it.
    //! \param[in] p1 1st point of the line.
    //! \param[in] p2 2nd point of the line.
    //! \note No check is made if p1 and p2 are supperposed.
    //-------------------------------------------------------------------------
    LineEquation(sf::Vector2f const& p1, sf::Vector2f const& p2)
    {
        // Line equation: a x + b y + c = 0
        a = p1.y - p2.y;
        b = p2.x - p1.x;
        c = -a * p1.x - b * p1.y;

        // Normalize line equation
        float n = std::sqrt(a * a + b * b);
        a /= n; b /= n; c /= n;
    }

    //-------------------------------------------------------------------------
    //! \brief Compute the distance between the given point and the line.
    //-------------------------------------------------------------------------
    float operator()(sf::Vector2f const& p)
    {
        return a * p.x + b * p.y + c;
    }

    float a, b, c;
};

static bool intersect1d(float a, float b, float c, float d)
{
    if (a > b)
        std::swap(a, b);
    if (c > d)
        std::swap(c, d);
    return std::max(a, c) <= (std::min(b, d) + EPS);
}

//-----------------------------------------------------------------------------
//! \brief Find the intersection of two segments.
//! \param[in] A the first vertice of segment AB.
//! \param[in] B the second vertice of segment AB.
//! \param[in] C the first vertice of segment CD.
//! \param[in] D the second vertice of segment CD.
//! \param[out] left the intersection coordinate if the return code is true.
//! \param[out] right the intersection coordinate if the return code is true.
//! \note If segments overlap, left and right are the bounding vertices. If
//! segments intersect then left and right are set with the same result. If
//! segments do not intersect they have undefined values.
//! \return 1 if the two segment intersect, return 2 if they overlap, else 0
//! if they do not intersect.
//-----------------------------------------------------------------------------
static size_t intersection(sf::Vector2f A, sf::Vector2f B, sf::Vector2f C,
                           sf::Vector2f D, sf::Vector2f& left, sf::Vector2f& right)
{
    // Bounding box
    if ((!intersect1d(A.x, B.x, C.x, D.x)) || (!intersect1d(A.y, B.y, C.y, D.y)))
        return 0u;

    // If two lines are not parallel, they intersect. To find their intersection
    // point, we need to solve the following system of linear equations:
    //   a1 x + b1 y + c1 = 0  // line equation of segment AB
    //   a2 x + b2 y + c2 = 0  // line equation of segment CD
    // Using the Cramer's rule (based on matrix determinant) the solution of
    // this system is:
    //   x = (c1 b2 - c2 b1) / (a1 b2 - a2 b1)
    //   y = (a1 c2 - a2 c1) / (a1 b2 - a2 b1)
    // If a1 b2 - a2 b1 == 0 then either the system has no solutions (the lines
    // are parallel and distinct) or there are infinitely solutions (the lines
    // overlap).  To distinguish these two cases, if both numerators equal  0
    // then lines overlap.
    LineEquation eqAB(A, B);
    LineEquation eqCD(C, D);
    double d = determinant(eqAB.a, eqAB.b, eqCD.a, eqCD.b);
    if (std::abs(d) < EPS)
    {
        // Parallel: no solution
        if ((std::abs(eqAB(C)) > EPS) || (std::abs(eqCD(A)) > EPS))
            return 0u;

        // Overlapping
        if (B < A)
            std::swap(A, B);
        if (D < C)
            std::swap(C, D);
        left = max(A, C);
        right = min(B, D);
        return 2u;
    }
    else
    {
        // Intersect ?
        left.x = right.x = -determinant(eqAB.c, eqAB.b, eqCD.c, eqCD.b) / d;
        left.y = right.y = -determinant(eqAB.a, eqAB.c, eqCD.a, eqCD.c) / d;
        return between(left.x, A.x, B.x) && between(left.y, A.y, B.y) &&
                between(left.x, C.x, D.x) && between(left.y, C.y, D.y) ? 1u : 0u;
    }
}

// ============================================================================
//! \brief Base class for road borders. Used for mathematical stuffs.
// ============================================================================
class Segment
{
public:

    // ========================================================================
    //! \brief Information about segments intersection (when happened).
    // ========================================================================
    struct Intersect
    {
        //! \brief Coordinate of the intersection
        sf::Vector2f position;
        //! \brief Offset from the initial vertices (in percent).
        float offset;
    };

public:

    Segment() = default;
    Segment(sf::Vector2f const& p1_, sf::Vector2f const& p2_)
        : p1(p1_), p2(p2_)
    {}

    bool intersection(Segment const& other, Segment::Intersect& intersec)
    {
        if (::intersection(p1, p2, other.p1, other.p2, intersec.position, intersec.position))
        {
            // Compute the offset (in percent) of the intersection from initial vertice
            sf::Vector2f AI = intersec.position - p1;
            sf::Vector2f AB = p2 - p1;
            intersec.offset = dot(AI, AB) / dot(AB, AB);
            return true;
        }
        return false;
    }

public:

    sf::Vector2f p1;
    sf::Vector2f p2;
};

// ============================================================================
//! \brief Road borders obtained after merging roads.
// ============================================================================
class RoadBorder: public Segment, public sf::Drawable
{
public:

    RoadBorder() = default;
    RoadBorder(sf::Vector2f const& p1, sf::Vector2f const& p2, float const width_)
        : Segment(p1, p2), width(width_)
    {
        // For debugging
        color = sf::Color(rand() % 255, rand() % 255, rand() % 255);
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        sf::RectangleShape shape(sf::Vector2f(distance(p1, p2), width));
        shape.setOrigin(0.0f, width / 2.0f);
        shape.setPosition(p1);
        shape.setRotation(orientation(p2, p1) * 180.0f / 3.1415f);
        shape.setFillColor(color);
        shape.setOutlineColor(color);
        shape.setOutlineThickness(1);

        target.draw(shape, states);
    }

public:

    mutable sf::Color color = sf::Color::Blue;
    float width;
};

std::ostream& operator<<(std::ostream& os, RoadBorder const& border)
{
    return os << "[" << border.p1 << ", " << border.p2 << "]";
}

// ============================================================================
//! \brief Road shape is a polygon with 2D capsule shape used for painting the
//! ground.
// ============================================================================
class RoadShape : public sf::Drawable
{
public:

    RoadShape() = default;

    //-------------------------------------------------------------------------
    //! \brief Initialize the shape of the road with a list of vertices (polygon)
    //-------------------------------------------------------------------------
    RoadShape(std::vector<sf::Vector2f> const& vertices)
    {
        m_shape.setPointCount(vertices.size());
        size_t i = 0u;
        for (auto const& it: vertices)
        {
            m_shape.setPoint(i++, it);
        }
        m_shape.setFillColor(sf::Color(140, 140, 140, 50));
        m_shape.setOutlineColor(sf::Color::Black);//(140, 140, 140, 255));
        m_shape.setOutlineThickness(1); //10
    }

    //-------------------------------------------------------------------------
    //! \brief Generate a 2D capsule (rectangle merged with 2 semi circles).
    //! \param[in] p1 origin point of the capsule, center of the 1st semi
    //! circle.
    //! \param[in] p2 destination point of the capsule, center of the 2nd semi
    //! circle.
    //! \param[in] width width of the rectangle.
    //! \param[in] roundness number of vertices for circles.
    //-------------------------------------------------------------------------
    static std::vector<sf::Vector2f> generateRoadEnvelopVertices(
        sf::Vector2f const& p1, sf::Vector2f const& p2,
        float const width, size_t const roundness)
    {
        const float PI = 3.14159265358979f;
        const float radius = width / 2.0f;
        const float alpha = orientation(p1, p2);
        const float alpha_cw = alpha + PI / 2.0f;
        const float alpha_ccw = alpha - PI / 2.0f;

        std::vector<sf::Vector2f> vertices;
        const float step = PI / float(std::max(size_t(1), roundness));
        const float eps = step / 2.0f;
        for (float i = alpha_ccw; i <= alpha_cw + eps; i += step)
        {
            vertices.push_back(translate(p1, i, radius));
        }
        for (float i = alpha_ccw; i <= alpha_cw + eps; i += step)
        {
            vertices.push_back(translate(p2, PI + i, radius));
        }

        return vertices;
    }

    // https://stackoverflow.com/questions/217578/how-can-i-determine-whether-a-2d-point-is-within-a-polygon
    // https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
    bool containsPoint(sf::Vector2f const& p)
    {
        sf::FloatRect rect = m_shape.getLocalBounds(); // m_shape.getGlobalBounds();
        if (!rect.contains(p))
            return false;

        bool inside = false;
        for (size_t i = 0, j = m_shape.getPointCount() - 1u; i < m_shape.getPointCount(); j = i++)
        {
            sf::Vector2f vi = m_shape.getPoint(i); // m_shape.getTransform().transformPoint(m_shape.getPoint(i));
            sf::Vector2f vj = m_shape.getPoint(j); // m_shape.getTransform().transformPoint(m_shape.getPoint(j));
            if (((vi.y > p.y) != (vj.y > p.y)) &&
                (p.x < (vj.x - vi.x) * (p.y - vi.y) /  (vj.y - vi.y) + vi.x))
            {
                inside = !inside;
            }
        }

        return inside;
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(m_shape, states);
    }

private:

    sf::ConvexShape m_shape;
};

// ============================================================================
class Road: public sf::Drawable
{
public:

    Road() = default;
    Road(sf::Vector2f const& p1, sf::Vector2f const& p2)
      : m_shape(RoadShape::generateRoadEnvelopVertices(p1, p2, 100.0f, 4u))
    {
        // FIXME en float const
        std::vector<sf::Vector2f> vertices =
            RoadShape::generateRoadEnvelopVertices(p1, p2, 100.0f, 4u);

        const size_t size = vertices.size();
        m_borders.reserve(size);
        for (size_t i = 1u; i <= size; i++)
        {
            m_borders.emplace_back(vertices[i - 1u], vertices[i % size], 2);
        }
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        m_shape.draw(target, states);
        for (auto const& it: m_borders)
        {
            //target.draw(it, states);
        }
    }

    inline std::vector<RoadBorder> const& borders() const
    {
        return m_borders;
    }

    bool containsSegment(Segment const& seg)
    {
        sf::Vector2f midpoint((seg.p1.x + seg.p2.x) / 2.0f,
                              (seg.p1.y + seg.p2.y) / 2.0f);
        return m_shape.containsPoint(midpoint);
    }

    static std::vector<sf::Vector2f> breakRoads(Road& road1, Road& road2)
    {
        std::vector<sf::Vector2f> intersections; // Debug only
        std::vector<RoadBorder>& borders1 = road1.m_borders;
        std::vector<RoadBorder>& borders2 = road2.m_borders;
        for (size_t i = 0u; i < borders1.size(); ++i)
        {
            for (size_t j = 0u; j < borders2.size(); ++j)
            {
                Segment::Intersect intersec;
                if (borders1[i].intersection(borders2[j], intersec))
                {
                    // Avoid an infinite loop by always adding, in the array we
                    // are iterating, same segments that are touching each other
                    // by a common vertex (i.e. previously split segments, for
                    // example ((0,0), (10,0)) and ((10,0), (20,0)).
                    // BUT: this may discard non-splitted segments.
                    if ((intersec.offset != 0.0f) && (intersec.offset != 1.0f))
                    {
                        intersections.push_back(intersec.position);

                        borders1.insert(borders1.begin() + i + 1u,
                            RoadBorder(intersec.position, borders1[i].p2, borders1[i].width));
                        borders2.insert(borders2.begin() + j + 1u,
                            RoadBorder(intersec.position, borders2[j].p2, borders2[j].width));
                        borders1[i].p2 = borders2[j].p2 = intersec.position;
                    }
                }
            }
        }

        return intersections;
    }

private:

    RoadShape m_shape;
    std::vector<RoadBorder> m_borders;
};

// ============================================================================
class Editor: public Application::GUI
{
public:

    Editor(Application& application)
        : Application::GUI(application, "Editor", sf::Color::White)//Color(50, 160, 40))
    {
        m_roads = {
            // FIXME: buggy on semicircles intersection
            /*Road{{100.0f, 100.0f}, {1000.0f, 100.0f}},
            Road{{100.0f, 100.0f}, {100.0f, 600.0f}},
            Road{{500.0f, 90.0f}, {100.0f, 600.0f}},*/

            Road{{100.0f, 500.0f}, {1000.0f, 500.0f}},
            Road{{200.0f, 600.0f}, {200.0f, 100.0f}},
            Road{{100.0f, 150.0f}, {1000.0f, 150.0f}},
            Road{{600.0f, 600.0f}, {600.0f, 100.0f}},
            Road{{100.0f, 100.0f}, {600.0f, 600.0f}},
        };

        breakAllRoads(m_roads);
        //m_borders = keptSegments(m_roads);
    }

private:

    void breakAllRoads(std::vector<Road>& roads)
    {
        for (size_t i = 0; i < roads.size() - 1u; i++)
        {
            for (size_t j = i + 1u; j < roads.size(); j++)
            {
                std::vector<sf::Vector2f> intersections = Road::breakRoads(roads[i], roads[j]);
                m_intersections.insert(m_intersections.end(), intersections.begin(), intersections.end());
            }
        }
    }

    // FIXME: au lieu de retourner les segments, supprimer ceux stockés dans road
    std::vector<RoadBorder> keptSegments(std::vector<Road>& roads)
    {
        std::vector<RoadBorder> segments;
        for (size_t i = 0u; i < roads.size(); i++)
        {
            for (auto const& border: roads[i].borders())
            {
                bool keep = true;
                for (size_t j = 0u; j < roads.size(); j++)
                {
                    if (i != j)
                    {
                        if (roads[j].containsSegment(border))
                        {
                            keep = false;
                            break;
                        }
                    }
                }
                if (keep) {
                    segments.push_back(border);
                }
            }
        }
        return segments;
    }

private:

    inline sf::Vector2f pixel2world(sf::Vector2i const& p)
    {
        return m_renderer.mapPixelToCoords(p);
    }

    virtual void onCreate() override
    {
        // SFML view: change the world coordinated to follow the same computations
        // than the doc "Estimation et controle pour le pilotage automatique de
        // vehicule" by Sungwoo Choi.
        m_view = m_renderer.getDefaultView();
        m_view.setSize(float(m_application.width()), -float(m_application.height()));
    }

    virtual void onUpdate(const float dt) override
    {}

    virtual void onDraw() override
    {
        for (auto const& it: m_roads)
        {
            m_renderer.draw(it);
        }

// FIXME temporaire
        for (auto const& it: m_borders)
        {
            m_renderer.draw(it);
        }

        for (auto const& it: m_intersections)
        {
            drawCircle(m_renderer, it);
        }
    }

    virtual void onHandleInput(sf::Event const& event) override
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            halt();
            break;
        case sf::Event::MouseButtonPressed:
            std::cerr << "Coordinates: "
               << pixel2world(sf::Mouse::getPosition(renderer()))
               << std::endl;
            break;
        default:
            break;
        }
    }

private:

    sf::View m_view;
    std::vector<Road> m_roads;
    std::vector<sf::Vector2f> m_intersections; // debug only
    std::vector<RoadBorder> m_borders; // A supprimer: devraait mettre a jour borders de m_roads
};

// ============================================================================
// g++ --std=c++14 Application.cpp RoadJunctions.cpp -o prog `pkg-config --cflags --libs sfml-graphics`
int main()
{
    srand(time(nullptr));

    Segment s1(sf::Vector2f(64, 137), sf::Vector2f(564, 637));
    Segment s2(sf::Vector2f(102, 450), sf::Vector2f(1001, 450));

    Segment::Intersect intersec;
    bool res = s1.intersection(s2, intersec);
    std::cout << res << std::endl;
    std::cout << intersec.offset << std::endl;
    std::cout << intersec.position.x << ", " << intersec.position.y << std::endl;

return 0;
#if 0
sf::Vector2f l, r;
res = Segment::intersect2(s1, s2, l, r);
    std::cout << res << std::endl;
    std::cout << l.x << ", " << l.y << std::endl;
    std::cout << r.x << ", " << r.y << std::endl;
    return 0;

std::vector<int> v{1, 2, 3};
v.insert(v.begin() + 3u, 42);
for (auto const& it: v)
{
    std::cout << it << " ";
}
std::cout << "\n";

    sf::Vector2f A(0.0f, 0.0f); sf::Vector2f B(20.0f, 0.0f);
    sf::Vector2f M(20.0f, 0.0f);
    sf::Vector2f AM = M - A; sf::Vector2f AB = B - A;
    std::cout << "offset: " << dot(AM, AB) / dot(AB, AB) << std::endl;
    return 0;
#endif

    Application application(1200, 720, "Editor");
    Editor editor(application);
    application.loop(editor);
    return 0;
}
