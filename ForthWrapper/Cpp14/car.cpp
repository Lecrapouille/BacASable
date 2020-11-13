/*
        car.cpp
        Miguel Leitao, 2012

        Vehicle simulation example using btosg abstraction layer.
        Can use a Z pointing up (default) or Y pointing (-D_UP_=0,1,0) up world coordinates reference.
        Vehicle can be interactively controlled from the keyboard:
                up: positive acceleration
                down: negative acceleration.
                right: steering right
                left: steering left
                b: brake
                ESC: quit

*/

//QQ
#include <map>
#include <ctype.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>


#include <osgViewer/Viewer>
#include <osg/Material>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>


#include <osg/Material>
#include <osg/Texture2D>

#define _DEBUG_ (0)

#include "btosgVehicle.h"

int ResetFlag=0;
double frame_time = 0.;

// Create World
btosgWorld myWorld;

btosgBox *myBox;

// QQ
// Basic stack
template<typename T>
class Stack
{
public:
    static constexpr size_t size = 16u; // max number of elements
    static constexpr size_t margin = 2u; // canari to detect stack over/under flow

    ~Stack() { std::cout << "Destroying Stack " << typeid(T).name() << std::endl; }

    inline void clear() { sp = sp0; }
    inline int32_t depth() const { return int32_t(sp - sp0); }
    inline void push(T const& t) { *(sp++) = t; }
    inline T& pop() { return *(--sp); }
    inline void drop() { --sp; }
    inline T& tos() { return *(sp - 1); }

    inline bool hasOverflowed() const { return sp > spM; }
    inline bool hasUnderflowed() const { return sp < sp0; }

private:

    T  data[size];
    T* const sp0 = data + margin;         // Begin of the stack
    T* const spM = data + size - margin;  // End of the stack

    T* sp = sp0;
};

// Container of heterogeneous stacks
class Stacks
{
public:

    template<class T> void clear() { stack<T>().clear(); }
    template<class T> int32_t depth() { return stack<T>().depth(); }
    template<class T> void push(T const& t) { stack<T>().push(t); }
    template<class T> T& pop() { return stack<T>().pop(); }
    template<class T> void drop() { stack<T>().drop(); }
    template<class T> T& tos() { return stack<T>().tos(); }
    template<class T> bool hasOverflowed() { return stack<T>().hasOverflowed(); }
    template<class T> bool hasUnderflowed() { return stack<T>().hasUnderflowed(); }

    // Destroy all created heterogeneous stacks
    void erase()
    {
        for (auto&& erase_func : m_erase_functions)
        {
            erase_func(*this);
        }
    }

    ~Stacks() { erase(); }

private:

    template<class T>
    Stack<T>& stack()
    {
        auto it = stacks<T>.find(this);
        if (it == std::end(stacks<T>))
        {
            // Hold list of created heterogeneous stacks for their destruction
            m_erase_functions.emplace_back([](Stacks& s)
            {
                std::cout << "Erasing stack " << typeid(T).name() << std::endl;
                stacks<T>.erase(&s);
            });

            return stacks<T>[this];
        }
        return it->second;
    }

    template<class T>
    static std::unordered_map<const Stacks*, Stack<T>> stacks;

    std::vector<std::function<void(Stacks&)>> m_erase_functions;
};

template<class T>
std::unordered_map<const Stacks*, Stack<T>> Stacks::stacks;


// QQ Forth
Stacks stacks;
using btosgDictionary = std::map<std::string, std::function<void()>>;
btosgDictionary dictionary;
int IP = 1; // Interpretation Pointer.
int g_argc = 0;
char** g_argv; // words to excute


// class to handle events
class EventHandler : public osgGA::GUIEventHandler
{
public:

    // QQ Remove global variable access
    void track(btosgVehicle* tracked) { m_tracked = tracked; }

    // QQ
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (m_tracked == nullptr) return false;

        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
        switch(ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYDOWN):
            switch ( ea.getKey() ) {
            case osgGA::GUIEventAdapter::KEY_Down:
                m_tracked->vehicle->applyEngineForce(-1500, 2);
                m_tracked->vehicle->applyEngineForce(-1500, 3);
                return false;
            case osgGA::GUIEventAdapter::KEY_Up:
                m_tracked->vehicle->applyEngineForce(1500, 2);
                m_tracked->vehicle->applyEngineForce(1500, 3);
                return false;
            case osgGA::GUIEventAdapter::KEY_Left:
                m_tracked->vehicle->setSteeringValue(btScalar(0.4), 0);
                m_tracked->vehicle->setSteeringValue(btScalar(0.4), 1);
                return false;
            case osgGA::GUIEventAdapter::KEY_Right:
                m_tracked->vehicle->setSteeringValue(btScalar(-0.4), 0);
                m_tracked->vehicle->setSteeringValue(btScalar(-0.4), 1);
                return false;
            case 'b':
            case '0':
            case osgGA::GUIEventAdapter::KEY_Control_R:
                m_tracked->vehicle->setBrake(10000, 2);
                m_tracked->vehicle->setBrake(10000, 3);
                return false;
            }
            break;
        case(osgGA::GUIEventAdapter::KEYUP):
            switch ( ea.getKey() ) {
            case osgGA::GUIEventAdapter::KEY_Down:
            case osgGA::GUIEventAdapter::KEY_Up:
                m_tracked->vehicle->applyEngineForce(0, 2);
                m_tracked->vehicle->applyEngineForce(0, 3);
                return false;
            case osgGA::GUIEventAdapter::KEY_Left:
            case osgGA::GUIEventAdapter::KEY_Right:
                m_tracked->vehicle->setSteeringValue(btScalar(0), 0);
                m_tracked->vehicle->setSteeringValue(btScalar(0), 1);
                return false;
            case '0':
            case 'b':
            case osgGA::GUIEventAdapter::KEY_Control_R:
                m_tracked->vehicle->setBrake(5, 2);
                m_tracked->vehicle->setBrake(5, 3);
                return false;
            case 'S':
                std::cout << "tecla S" << std::endl;
                return false;
            case 'i':
                m_tracked->printInfo();
                break;
            case 'f':
                std::cout << "adding force" << std::endl;
                myBox->body->activate(true);
                myBox->body->applyCentralImpulse(btVector3(100.,0.,0.));
                return false;
            case 'F':
                std::cout << "adding Force" << std::endl;

                m_tracked->vehicle->applyEngineForce(500, 2);
                m_tracked->vehicle->applyEngineForce(500, 3);

                int i;
                for( i=0 ; i<m_tracked->vehicle->getNumWheels() ; i++) {
                    btWheelInfo& iWheel = m_tracked->vehicle->getWheelInfo(i);
                    printf(" wheel %d, radius %f, rotation %f, eforce %f, steer %f\n",
                           i, iWheel.m_wheelsRadius, iWheel.m_rotation, iWheel.m_engineForce,iWheel.m_steering);
                }
                // handled = true;
                return false;
            case 'R':
            case 'r':
                ResetFlag = 1;
                std::cout << "tecla R" << std::endl;
                break;
            }
            break;
        case(osgGA::GUIEventAdapter::MOVE):
            //std::cout << "mouse move " << ea.getX() << " " << ea.getY() << std::endl;
            return false;
        default:
            return false;
        }
        return true;
    }

private:

    btosgVehicle* m_tracked = nullptr; // QQ should be btosgObject*
};


class BlockGreen : public btosgBox {
public:
    BlockGreen(float x, float y, float z) : btosgBox( osg::Vec3(1.,1.,1.), 100. ) {
        setPosition(btVector3(x,y,z));
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.1, 0.5, 0.1, 1.0));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 64);
        model->getOrCreateStateSet()->
        setAttributeAndModes(mat, osg::StateAttribute::ON);
    }
    BlockGreen(float x, float z) : BlockGreen(x,3.,z) {};
};

class BlockRed : public btosgBox {
public:
    BlockRed(float x, float y, float z) : btosgBox( osg::Vec3(1.,1.,1.), 5000. ) {
        setPosition(btVector3(x,y,z));
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.1, 0.1, 1.0));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 64);
        model->getOrCreateStateSet()->
        setAttributeAndModes(mat, osg::StateAttribute::ON);
    }
    BlockRed(float x, float z) : BlockRed(x,3.,z) {};
};

class BlockBlue : public btosgBox {
public:
    BlockBlue(float x, float y, float z) : btosgBox( osg::Vec3(1.,0.25,1.), 10000. ) {
        setPosition(btVector3(x,y,z));
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setAmbient (osg::Material::FRONT_AND_BACK, osg::Vec4(0., 0., 0., 1.0));
        mat->setDiffuse (osg::Material::FRONT_AND_BACK, osg::Vec4(0.1, 0.1, 0.5, 1.0));
        mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1.0));
        mat->setShininess(osg::Material::FRONT_AND_BACK, 64);
        model->getOrCreateStateSet()->
        setAttributeAndModes(mat, osg::StateAttribute::ON);
    }
    BlockBlue(float x, float z) : BlockBlue(x,3.,z) {};
};

// QQ Forth primitives
static void createCar()
{
    stacks.push<btosgObject*>(new btosgVehicle(&myWorld));
    myWorld.addObject(stacks.tos<btosgObject*>());
}

static void setMass()
{
    stacks.tos<btosgObject*>()->setMass(stacks.pop<double>());
}

static void setName()
{
    stacks.tos<btosgObject*>()->setName(g_argv[++IP]);
}

static void times()
{
    stacks.tos<double>() *= stacks.pop<double>();
}

static void add()
{
    stacks.tos<double>() += stacks.pop<double>();
}

static void UpCoord()
{
    stacks.push<double>(0.0); // x
    stacks.push<double>(0.0); // y
    stacks.push<double>(1.0); // z
}

static void setCoord()
{
    double x = stacks.pop<double>();
    double y = stacks.pop<double>();
    double z = stacks.pop<double>();
    stacks.tos<btosgObject*>()->setPosition(x, y, z);
}

// QQ Create Forth dico
void create_dictionary(btosgDictionary& dictionary)
{
    dictionary =
    {
        { "car:", createCar },
        { "setMass", setMass },
        { "setName", setName },
        { "up", UpCoord },
        { "coord", setCoord },
        { "*", times },
        { "+", add },
    };
}

static bool isInteger(std::string const& word, int& val)
{
    bool isNumber = true;
    for(std::string::const_iterator k = word.begin(); k != word.end(); ++k)
        isNumber = (isNumber && isdigit(*k));

    if (isNumber)
        val = strtol(word.c_str(), nullptr, 10);

    return isNumber;
}

// QQ Interpret Forth script
bool parse(btosgDictionary& dictionary, int argc, char* argv[])
{
    g_argc = argc;
    g_argv = argv;
    int number;

    std::cout << "Running command line script:" << std::endl;
    for (IP = 1; IP < argc; ++IP)
    {
        const char* word = argv[IP];

        std::cout << "Forth: Execute word '" << word << "'" << std::endl;
        auto it = dictionary.find(word);
        if (it != dictionary.end())
        {
            it->second();
        }
        else if (isInteger(word, number))
        {
            std::cout << "Is int ? " << word << std::endl;
            stacks.push<double>(number);
        }
        else
        {
            std::cerr << "Forth: Unknown word '" << word
                      << "'. Abort !" << std::endl;
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    btosgVec3 up(0., 0., 1.);
    btosgVec3 gravity = up*-9.8;
    myWorld.dynamic->setGravity(gravity);

    // QQ
    if (argc == 1)
    {
        std::cerr << "You did not gave a forth script in the command line" << std::endl;
        std::cerr << "Example:" << std::endl;
        std::cerr << "   car: 800 setMass setName MyCar up 3 \"*\" coord" << std::endl;
        return EXIT_FAILURE;
    }

    // QQ
    create_dictionary(dictionary);
    if (!parse(dictionary, argc, argv))
        return EXIT_FAILURE;

    // QQ
    // Car
    //stacks.push<btosgObject*>(new btosgVehicle(&myWorld));
    btosgObject* obj = stacks.tos<btosgObject*>();
    if (obj == nullptr)
    {
        std::cerr << "stack underflow" << std::endl;
        return EXIT_FAILURE;
    }

    btosgVehicle* myVehicle = dynamic_cast<btosgVehicle*>(obj);
    if (myVehicle == nullptr)
    {
        std::cerr << "bad dynamic cast" << std::endl;
        return EXIT_FAILURE;
    }

    //myVehicle->setPosition(btosgVec3(up*3.));
    //myVehicle->setName("Vehicle");
    //myVehicle->setMass(800.);
    //myWorld.addObject( myVehicle );

    // Plane
    btosgPlane *myRamp = new btosgPlane(osg::Vec3(50.,50.,50.) - up*50.);
    myRamp->setPosition(0.,0.,0.);
    myRamp->setName("Ramp");
    myRamp->body->setFriction(100.);
    myWorld.addObject( myRamp );

    // Creating the viewer
    osgViewer::Viewer viewer ;

    // Setup camera
    osg::Matrix matrix;
    matrix.makeLookAt( osg::Vec3(0.,8.,5.), osg::Vec3(0.,0.,1.), up );
    viewer.getCamera()->setViewMatrix(matrix);

    // add the Event handler
    EventHandler* eh = new EventHandler();
    eh->track(myVehicle);
    viewer.addEventHandler(eh);
    viewer.setSceneData( myWorld.scene );

    osg::ref_ptr<osgGA::TrackballManipulator> manipulator = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator( manipulator );

    // Set the desired home coordinates for the manipulator
    osg::Vec3d eye(osg::Vec3(0., -5., -5.)+up*20.);
    osg::Vec3d center(0., 0., 0.);

    // Make sure that OSG is not overriding our home position
    manipulator->setAutoComputeHomePosition(false);
    // Set the desired home position of the Trackball Manipulator
    manipulator->setHomePosition(eye, center, up);
    // Force the camera to move to the home position
    manipulator->home(0.0);

    // record the timer tick at the start of rendering.
    osg::Timer myTimer;
    double timenow = myTimer.time_s();
    double last_time = timenow;
    frame_time = 0.;

    while( !viewer.done() )
    {
        myWorld.stepSimulation(frame_time,10);
        viewer.frame();
        timenow = myTimer.time_s();
        frame_time = timenow - last_time;
        last_time = timenow;

        if (ResetFlag>0) {
            myWorld.reset();
            ResetFlag = 0;
        }
    }

    return EXIT_SUCCESS;
}
