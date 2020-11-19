#include "bsplinecurve.h"
#include "trophy.h"
#include "gsl_math.h"

BSplineCurve::BSplineCurve()
{
    trophyPoints.push_back(gsl::Vector3D(7, 2.7f, 1.7f));
    trophyPoints.push_back(gsl::Vector3D(-11, 4.12f, -15.f));
    trophyPoints.push_back(gsl::Vector3D(-0, 10.6f, -19));

    createDefaultSplineCurve();
    addCurveToVertices(50);
    createTrophies();
}

BSplineCurve::BSplineCurve(GLint mMatrixUniform, RenderWindow *inRenderWindow)
{
    currentRenderWindow = inRenderWindow;
    trophyPoints.push_back(gsl::Vector3D(7, 2.7f, 1.7f));
    trophyPoints.push_back(gsl::Vector3D(-11, 4.12f, -15.f));
    trophyPoints.push_back(gsl::Vector3D(-0, 10.6f, -19));

    mMatrixUniformTrophy = mMatrixUniform;

    createDefaultSplineCurve();
    addCurveToVertices(50);

    isTrophyPickedUp.resize(3);

    createTrophies();
}

BSplineCurve::~BSplineCurve()
{
    glDeleteVertexArrays( 1, &mVAO );
    glDeleteBuffers( 1, &mVBO );

    for (auto trophy : trophies)
    {
        delete trophy;
    }
}

void BSplineCurve::updateSpline()
{
    int scaleNum = 5;

    newControlPoints.clear();

    newControlPoints.push_back(gsl::Vector3D(-4 * scaleNum, 0, -4 * scaleNum));
    newControlPoints.push_back(gsl::Vector3D(-3 * scaleNum, 0, -4 * scaleNum));

    if (!isTrophyPickedUp[0])
    {
        newControlPoints.push_back(gsl::Vector3D(7,  0, 1.7f));
    }
    else if (!isTrophyPickedUp[1])
    {
        newControlPoints.push_back(gsl::Vector3D(-11,  0,  -15));
    }
    else if (!isTrophyPickedUp[2])
    {
        newControlPoints.push_back(gsl::Vector3D(-0, 0, -19));
    }

    newControlPoints.push_back(gsl::Vector3D( -3 * scaleNum, 0,  4 * scaleNum));
    newControlPoints.push_back(gsl::Vector3D( -4 * scaleNum, 0,  4 * scaleNum));

    createClampedKnots(degree, newControlPoints.size());
}

void BSplineCurve::addNewSpline()
{
    controlPoints.clear();
    controlPoints = newControlPoints;
    newPathChange = false;
    addCurveToVertices(50);
}

void BSplineCurve::createDefaultSplineCurve()
{
    int scaleNum = 5;

    controlPoints.push_back(gsl::Vector3D(-4 * scaleNum, 0, -4 * scaleNum));
    controlPoints.push_back(gsl::Vector3D(-3 * scaleNum, 0, -4 * scaleNum));

    //trophy coordinates //replace with a new trophy function
    createTrophies();
    controlPoints.push_back(gsl::Vector3D(7,  0, 1.7f));
    controlPoints.push_back(gsl::Vector3D(-11,  0,  -15));
    controlPoints.push_back(gsl::Vector3D(-0, 0, -19));

    controlPoints.push_back(gsl::Vector3D( -3 * scaleNum, 0,  4 * scaleNum));
    controlPoints.push_back(gsl::Vector3D( -4 * scaleNum, 0,  4 * scaleNum));


    createClampedKnots(degree, controlPoints.size());
}

void BSplineCurve::addCurveToVertices(int subdivisions)
{
    mVertices.clear();
    gsl::Vector3D temp;
    Vertex tempVertex;



    for (int i = 0; i < subdivisions + 2; ++i)
    {
        temp = gsl::bSpline(controlPoints, knots, i/static_cast<float>(subdivisions), degree);
        tempVertex.set_xyz(temp);
        mVertices.push_back(tempVertex);

        //qDebug() << i/static_cast<float>(subdivisions);
    }

}

std::vector<gsl::Vector3D> BSplineCurve::getSplineVerticeLocations()
{
    std::vector<gsl::Vector3D> temp;

    for (auto vertex : mVertices)
    {
        temp.push_back(vertex.get_xyz());
    }
    return temp;
}

void BSplineCurve::setNewHeights(std::vector<float> inFloats)
{

    for (int i = 0; i < mVertices.size(); ++i)
    {
        mVertices[i].set_xyz(mVertices[i].get_xyz().getX(), inFloats[i], mVertices[i].get_xyz().getZ());
    }
}

gsl::Vector3D BSplineCurve::getCurrentSplineLocation(float t)
{
     auto tempFloat = gsl::bSpline(controlPoints, knots, t, degree);
     return tempFloat;
}

void BSplineCurve::createTrophies()
{
    Trophy* tempTrophy1 = new Trophy;
    Trophy* tempTrophy2 = new Trophy;
    Trophy* tempTrophy3 = new Trophy;

    trophies.clear(); //Clear before creation

    tempTrophy1->mMatrix.setPosition(7, 2.7f/2.f, 1.7f);
    trophies.push_back(tempTrophy1);
    qDebug() << "trophy 1 is" << tempTrophy1->mMatrix.getPosition();

    tempTrophy2->mMatrix.setPosition(-11, 8.12f/2.f, -15);
    trophies.push_back(tempTrophy2);
    qDebug() << "trophy 2 is" << tempTrophy2->mMatrix.getPosition();

    tempTrophy3->mMatrix.setPosition(0, 10.6f, -19);
    trophies.push_back(tempTrophy3);
    qDebug() << "trophy 3 is" << tempTrophy3->mMatrix.getPosition();


    for (auto trophy : trophies)
    {
        trophy->init();
    }
}

void BSplineCurve::createClampedKnots(int degree, int numberOfControlPoints)
{
    knots.clear();

    for (int i = 0; i < degree + 1; ++i)
    {
        knots.push_back(0);
    }

    if (numberOfControlPoints > degree)
    {
        for (int i = 0; i < numberOfControlPoints - (degree + 1); ++i)
        {
            float tempPiece = 1.f / (numberOfControlPoints - degree);
            knots.push_back(tempPiece * ((i+1)));
        }
    }

    for (int i = 0; i < degree + 1; ++i)
    {
        knots.push_back(1);
    }

//    for (auto knot : knots)
//    {
//        qDebug() << knot;
//    }
}

void BSplineCurve::eventEndOfSpline()
{
    //qDebug() << "End of spline hit";
    if (newPathChange)
    {
        updateSpline();
        addNewSpline();
        currentRenderWindow->updateSplineHeight(this);
        init();
    }
}

void BSplineCurve::init()
{
    initializeOpenGLFunctions();

    //Vertex Array Object - VAO
    glGenVertexArrays( 1, &mVAO );
    glBindVertexArray( mVAO );

    //Vertex Buffer Object to hold vertices - VBO
    glGenBuffers( 1, &mVBO );
    glBindBuffer( GL_ARRAY_BUFFER, mVBO );

    glBufferData( GL_ARRAY_BUFFER, mVertices.size()*sizeof(Vertex), mVertices.data(), GL_STATIC_DRAW );

    // glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE,sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  sizeof(Vertex),  (GLvoid*)(3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    //glLineWidth(3);
    glPointSize(5);
}

void BSplineCurve::draw()
{
    glBindVertexArray( mVAO );
    glDrawArrays(GL_LINE_STRIP, 0, mVertices.size());

    /*for (unsigned int i = 0; i < trophies.size(); ++i)
    {
        if (isTrophyPickedUp[i])
        {
        }
        else
        {
            trophies[i]->draw(mMatrixUniformTrophy);
        }
    }*/
}

bool BSplineCurve::getIsTrophyPickedUp(unsigned int index)
{
    return isTrophyPickedUp[index];
}

void BSplineCurve::setTrophyStatus(bool isPicked, unsigned int index)
{
    isTrophyPickedUp[index] = isPicked;
    newPathChange = true;

//    for (auto trophyPick : isTrophyPickedUp)
//    {
//        qDebug() << trophyPick;
//    }
}

std::vector<gsl::Vector3D> BSplineCurve::getTrophyLocations()
{
    return trophyPoints;
}
