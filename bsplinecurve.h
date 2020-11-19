#ifndef BSPLINECURVE_H
#define BSPLINECURVE_H
#include "visualobject.h"
#include "vector3d.h"
#include "renderwindow.h"

class Trophy;
class BSplineCurve : public VisualObject
{
public:
    BSplineCurve();
    BSplineCurve(GLint mMatrixUniform, RenderWindow* inRenderWindow);
    ~BSplineCurve() override;

    void updateSpline();
    void addNewSpline();

    bool newPathChange{false};

    void createDefaultSplineCurve();
    void addCurveToVertices(int subdivisions);
    std::vector<gsl::Vector3D> getSplineVerticeLocations();
    void setNewHeights(std::vector<float> inFloats);
    gsl::Vector3D getCurrentSplineLocation(float t);

    void createTrophies();

    void createClampedKnots(int degree, int numberOfControlPoints);
    void eventEndOfSpline();

    virtual void init() override;
    virtual void draw() override;

    RenderWindow* currentRenderWindow;

    std::vector<Trophy*> trophies;

    std::vector<bool> isTrophyPickedUp;
    bool getIsTrophyPickedUp(unsigned int index);
    void setTrophyStatus(bool isPicked, unsigned int index);

    std::vector<gsl::Vector3D> trophyPoints;
    std::vector<gsl::Vector3D> getTrophyLocations();

    GLint mMatrixUniformTrophy;



private:
    std::vector<gsl::Vector3D> controlPoints;      // control points
    std::vector<gsl::Vector3D> newControlPoints;      // control points
    std::vector<GLfloat> knots;                    // knots
    unsigned long long degree{3};                                         //d = degree

};

#endif // BSPLINECURVE_H
