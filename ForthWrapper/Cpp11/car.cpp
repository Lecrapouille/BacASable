/*
Code based on the original code https://github.com/miguelleitao/btosg
and modified to add a very basic Forth wrapper on btosg library (grep QQ tags).
Compilation: Replace the file btosg/tree/master/examples/car.cpp but his one
and use the btosg Makefile for compiling examples.
Run type on you console the following command:
./carY car: 800 setMass setName MyCar up 3 "*" coord


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
#include <functional>
#include <map>
#include <ctype.h>

// QQ Reuse my stack class form my personal Forth interpreter
#include "Stack.hpp"

// QQ Dirty extension of the base stack to use pointers.
// Dynamic cast is used to avoid bad casting. You can replace
// assert by throw.
namespace forth
{
template<typename T>
class ExtStack: public Stack<T>
{
public:

    ExtStack(const char *name)
        : Stack<T> (name)
    {}

    // Remove and return the top of stack element.
    template<class U>
    U pop_ptr()
    {
        U ptr = dynamic_cast<U>(*(--Stack<T>::sp));
        assert((ptr != nullptr) && "pop: Incompatible dynamic cast");
        return ptr;
    }

    // Return the top of stack element (TOS).
    template<class U>
    U tos_ptr()
    {
        U ptr = dynamic_cast<U>(*(Stack<T>::sp - 1));
        assert((ptr != nullptr) && "pick: Incompatible dynamic cast");
        return ptr;
    }

    // Pick the nth element from TOS.
    template<class U>
    U pick_ptr(int const nth)
    {
        U ptr = dynamic_cast<U>(*(Stack<T>::sp - nth - 1));
        assert((ptr != nullptr) && "tos: Incompatible dynamic cast");
        return ptr;
    }
};
}

// QQ Basic Forth
forth::ExtStack<btosgObject*> OS("btosgObject stack");
forth::Stack<double> DS("data stack");
using btosgDictionary = std::map<std::string, std::function<void()>>;
btosgDictionary dictionary;
int IP = 1; // Interpretation Pointer.
char** g_argv; // Words to excute for command line


// class to handle events
// QQ modified because in the original code it directly use a global variable
class EventHandler : public osgGA::GUIEventHandler
{
public:

    // QQ Remove global variable access
    void track(btosgVehicle* tracked) { m_tracked = tracked; }

    // QQ modified to not track a global variable
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
    OS.push(new btosgVehicle(&myWorld));
    myWorld.addObject(OS.tos());
}

static void setMass()
{
    OS.tos()->setMass(DS.pop());
}

static void setName()
{
    OS.tos()->setName(g_argv[++IP]);
}

static void times()
{
    DS.tos() *= DS.pop();
}

static void add()
{
    DS.tos() += DS.pop();
}

static void UpCoord()
{
    DS.push(0.0); // x
    DS.push(0.0); // y
    DS.push(1.0); // z
}

static void setCoord()
{
    double x = DS.pop();
    double y = DS.pop();
    double z = DS.pop();
    OS.tos()->setPosition(x, y, z);
}

// QQ Create basic Forth dictionary
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

// QQ is the given string (word) a number ? If yes return its value + true
static bool isInteger(std::string const& word, int& val)
{
    bool isNumber = true;
    for(std::string::const_iterator k = word.begin(); k != word.end(); ++k)
        isNumber = (isNumber && isdigit(*k));

    if (isNumber)
        val = strtol(word.c_str(), nullptr, 10);

    return isNumber;
}

// QQ Interpret a Forth script given on the command line
bool parse(btosgDictionary& dictionary, int argc, char* argv[])
{
    g_argv = argv; // global access
    int number;

    std::cout << "Running command line script:" << std::endl;
    for (IP = 1; IP < argc; ++IP)
    {
        const char* word = argv[IP];

        std::cout << "Forth: Execute word '" << word << "'" << std::endl;
        auto it = dictionary.find(word);
        if (it != dictionary.end())
        {
            it->second(); // execute its definition
        }
        else if (isInteger(word, number))
        {
            std::cout << "Is int ? " << word << std::endl;
            DS.push(number);
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
    create_dictionary(dictionary);

    // QQ interpret the argv. Example:
    // "car:" "800" "setMass" "setName" "MyCar" "up" "3" "*" "coord"
    if (!parse(dictionary, argc, argv))
        return EXIT_FAILURE;

    // QQ Create a car.
    // The following script "car: 800 setMass setName MyCar up 3 * coord"
    // will do the same thing that commented lines.

    //OS.push(new btosgVehicle(&myWorld));
    btosgVehicle *myVehicle = OS.tos_ptr<btosgVehicle*>();
    //myVehicle->setPosition(btosgVec3(up*3.));
    //myVehicle->setName("Vehicle");
    //myVehicle->setMass(800.);
    //myWorld.addObject(myVehicle);

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
    eh->track(myVehicle); // QQ track the desired vehicle
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
