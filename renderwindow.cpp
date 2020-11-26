#include "renderwindow.h"
#include <QTimer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLDebugLogger>
#include <QKeyEvent>
#include <QStatusBar>
#include <QDebug>

#include "shader.h"
#include "mainwindow.h"
#include "matrix4x4.h"
#include "gsl_math.h"
#include "LAS/lasloader.h"

#include "xyz.h"
#include "trianglesurface.h"
#include "octahedronball.h"
#include "beziercurve.h"
#include "bsplinecurve.h"
#include "rollingball.h"

RenderWindow::RenderWindow(const QSurfaceFormat &format, MainWindow *mainWindow)
    : mContext(nullptr), mInitialized(false), mMainWindow(mainWindow)
{
    //This is sent to QWindow:
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
    //Make the OpenGL context
    mContext = new QOpenGLContext(this);
    //Give the context the wanted OpenGL format (v4.1 Core)
    mContext->setFormat(requestedFormat());
    if (!mContext->create()) {
        delete mContext;
        mContext = nullptr;
        qDebug() << "Context could not be made - quitting this application";
    }

    //Make the gameloop timer:
    mRenderTimer = new QTimer(this);
}

RenderWindow::~RenderWindow()
{
}


void RenderWindow::updateSplineHeight(VisualObject *temp)
{
    std::vector<gsl::Vector3D> tempLocs = static_cast<BSplineCurve*>(temp)->getSplineVerticeLocations();
    std::vector<float> tempHeights;
    float tempLoc;
    for (auto locs : tempLocs)
    {
        tempLoc = getTerrainHeight(gsl::Vector3D(locs.getX(), 0.f, locs.getZ()));
        tempHeights.push_back(tempLoc + 0.5f);
    }
    static_cast<BSplineCurve*>(temp)->setNewHeights(tempHeights);
}

/// Sets up the general OpenGL stuff and the buffers needed to render a triangle
void RenderWindow::init()
{
    //Connect the gameloop timer to the render function:
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(render()));

    //********************** General OpenGL stuff **********************

    //The OpenGL context has to be set.
    //The context belongs to the instanse of this class!
    if (!mContext->makeCurrent(this)) {
        qDebug() << "makeCurrent() failed";
        return;
    }

    //just to make sure we don't init several times
    //used in exposeEvent()
    if (!mInitialized)
        mInitialized = true;

    //must call this to use OpenGL functions
    initializeOpenGLFunctions();

    //Start the Qt OpenGL debugger
    //Really helpfull when doing OpenGL
    //Supported on most Windows machines
    //reverts to plain glGetError() on Mac and other unsupported PCs
    // - can be deleted
    startOpenGLDebugger();

    //general OpenGL stuff:
    glEnable(GL_DEPTH_TEST);    //enables depth sorting - must use GL_DEPTH_BUFFER_BIT in glClear
    //glEnable(GL_CULL_FACE);     //draws only front side of models - usually what you want -
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);    //color used in glClear GL_COLOR_BUFFER_BIT

    //Compile shaders:
    //NB: hardcoded path to files! You have to change this if you change directories for the project.
    //Qt makes a build-folder besides the project folder. That is why we go down one directory
    // (out of the build-folder) and then up into the project folder.
    mShaderProgram[0] = new Shader("../VSIM_Folder3/plainvertex.vert", "../VSIM_Folder3/plainfragment.frag");
    qDebug() << "Plain shader program id: " << mShaderProgram[0]->getProgram();
    mShaderProgram[1]= new Shader("../VSIM_Folder3/texturevertex.vert", "../VSIM_Folder3/texturefragmet.frag");
    qDebug() << "Texture shader program id: " << mShaderProgram[1]->getProgram();

    mShaderProgram[2] = new Shader("../VSIM_Folder3/NPCVertex.vert", "../VSIM_Folder3/NPCfragment.frag");
    qDebug() << "Plain shader program id: " << mShaderProgram[2]->getProgram();

    setupPlainShader(0);
    setupTextureShader(1);

    //**********************  Texture stuff: **********************
    mTexture[0] = new Texture();
    mTexture[1] = new Texture("../VSIM_Folder3/Assets/hund.bmp");

    //Set the textures loaded to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture[0]->id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTexture[1]->id());

    //********************** Making the objects to be drawn **********************
    VisualObject *temp = new XYZ();
    temp->init();
    mVisualObjects.push_back(temp);

    //Initialize terrain
 initTerrain();
    //testing triangle surface class
//    temp = new TriangleSurface();
//    temp->init();
//    mVisualObjects.push_back(temp);

    //Oppgave 2:
    mSurface = new TriangleSurface();
    mSurface->init();

    mVisualObjects.push_back(mSurface);

    mBall = new RollingBall(6);
    mBall->init();
    mBall->mMatrix.scale(0.25);
    //mBall->mMatrix.rotateX(90);
    //Her blir ballen sluppet over bakken
    mBall->mMatrix.setPosition(0.f,12.f,0.f);
    mBall->startPos = mBall->mMatrix.getPosition();
    mBall->mAcceleration = gsl::vec3{0.f,-9.81f,0.f};
    mVisualObjects.push_back(mBall);

    //controllable player
    mPlayer = new RollingBall(6);
    mPlayer->init();
    mPlayer->mMatrix.setPosition(0, 5.f, 0);
    mPlayer->startPos = mPlayer->mMatrix.getPosition();
    mPlayer->mAcceleration = gsl::vec3{0.f, -9.81f, 0.f};
    mVisualObjects.push_back(mPlayer);

    //Curve
    mCurve = new BSplineCurve(mMatrixUniform0, this);
    ////Visualize curve height------------------
    updateSplineHeight(mCurve);
    ////Visualize end------------------
    mCurve->init();
    trophyPositions = static_cast<BSplineCurve*>(mCurve)->getTrophyLocations();
    mVisualObjects.push_back(mCurve);

    //NPC ball
    mNPC = new OctahedronBall{3};
    mNPC->init();
    mNPC->mMatrix.setPosition(0, 5.f, 0);
    mNPC->startPos = mNPC->mMatrix.getPosition();
    mNPC->mAcceleration = gsl::vec3{0.f, -9.81f, 0.f};
    mVisualObjects.push_back(mNPC);



    //********************** Set up camera **********************
    mCurrentCamera = new Camera();
    mCurrentCamera->setPosition(gsl::Vector3D(30.f, -20.5f, 15.f));
    mCurrentCamera->yaw(45);
    mCurrentCamera->pitch(20);



}

///Called each frame - doing the rendering
void RenderWindow::render()
{

    //input
    handleInput();

    mCurrentCamera->update();

    const float deltaTime = mTimeStart.nsecsElapsed() / 1000000000.f;
    mTimeStart.restart(); //restart FPS clock
    mContext->makeCurrent(this); //must be called every frame (every time mContext->swapBuffers is called)

    //to clear the screen for each redraw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //******** This should be done with a loop!
    {

        glUseProgram(mShaderProgram[0]->getProgram());
        glUniformMatrix4fv( vMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform0, 1, GL_TRUE, mVisualObjects[0]->mMatrix.constData());
        mVisualObjects[0]->draw();

        glUseProgram(mShaderProgram[0]->getProgram());
//        gsl::Matrix4x4 modelMat{};
//        modelMat.setToIdentity();
        glUniformMatrix4fv(mShaderProgram[0]->vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( mShaderProgram[0]->pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mShaderProgram[0]->mMatrixUniform, 1, GL_TRUE, mTerrainModMat.constData());
        glBindVertexArray(mTerrainVAO);
        // glDrawArrays(GL_POINTS, 0, mTerrainVertices.size());
        glDrawElements(GL_TRIANGLES, mTerrainTriangles.size() * 3, GL_UNSIGNED_INT, 0);

        //Oppgave 2:
        /*glUseProgram(mShaderProgram[0]->getProgram());
        glUniformMatrix4fv( vMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform0, 1, GL_TRUE, mVisualObjects[1]->mMatrix.constData());
        mVisualObjects[1]->draw();*/


        glUseProgram(mShaderProgram[0]->getProgram());
        moveBall(deltaTime);
        //mVisualObjects[2]->update(deltaTime);
        glUniformMatrix4fv( vMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform0, 1, GL_TRUE, mVisualObjects[2]->mMatrix.constData());
        mVisualObjects[2]->draw();

        glUseProgram(mShaderProgram[0]->getProgram());
        glUniformMatrix4fv( vMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform0, 1, GL_TRUE, mVisualObjects[3]->mMatrix.constData());
        mVisualObjects[3]->draw();

        glUseProgram(mShaderProgram[0]->getProgram());
        glUniformMatrix4fv( vMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform0, 1, GL_TRUE, mVisualObjects[4]->mMatrix.constData());
        mVisualObjects[4]->draw();

        glUseProgram(mShaderProgram[0]->getProgram());
        glUniformMatrix4fv( vMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( pMatrixUniform0, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glUniformMatrix4fv( mMatrixUniform0, 1, GL_TRUE, mVisualObjects[5]->mMatrix.constData());
        mVisualObjects[5]->draw();



    }

    moveBallAlongSpline(static_cast<BSplineCurve*>(mVisualObjects[4]), mVisualObjects[5]);




    //Is player < 3 away from trophy?
    checkIfPlayerIsCloseToTrophy();



    //Calculate framerate before
    // checkForGLerrors() because that takes a long time
    // and before swapBuffers(), else it will show the vsync time
    calculateFramerate();

    //using our expanded OpenGL debugger to check if everything is OK.
    checkForGLerrors();

    //Qt require us to call this swapBuffers() -function.
    // swapInterval is 1 by default which means that swapBuffers() will (hopefully) block
    // and wait for vsync.
    mContext->swapBuffers(this);
}

std::vector<gsl::Vector3D> RenderWindow::mapToGrid(const std::vector<gsl::Vector3D>& points, int xGrid, int zGrid, gsl::Vector3D min, gsl::Vector3D max)
{
    std::vector<std::pair<float, unsigned int>> grid;
    grid.resize(xGrid * zGrid);

    for (auto point : points)
    {
        int closestIndex[2]{0, 0};
        for (int z{0}; z < zGrid; ++z)
        {
            for (int x{0}; x < xGrid; ++x)
            {
                gsl::Vector3D gridPoint{
                    x * ((max.x - min.x) / xGrid) + min.x,
                    0,
                    z * ((max.z - min.z) / zGrid) + min.z
                };

                gsl::Vector3D lastClosestPoint{
                    closestIndex[0] * ((max.x - min.x) / xGrid) + min.x,
                    0,
                    closestIndex[1] * ((max.z - min.z) / zGrid) + min.z
                };

                if ((gsl::Vector3D{point.x, 0, point.z} - gridPoint).length() < (gsl::Vector3D{point.x, 0, point.z} - lastClosestPoint).length())
                {
                    closestIndex[0] = x;
                    closestIndex[1] = z;
                }
            }
        }

        auto& p = grid.at(closestIndex[0] + closestIndex[1] * zGrid);
        p.first += point.y;
        ++p.second;
    }

    // convert pair into only first of pair
    std::vector<gsl::Vector3D> outputs{};
    outputs.reserve(grid.size());
    for (int z{0}; z < zGrid; ++z)
    {
        for (int x{0}; x < xGrid; ++x)
        {
            auto &p = grid.at(x + z * zGrid);

            outputs.emplace_back(x * ((max.x - min.x) / xGrid) + min.x,
                                 (0 < p.second) ? p.first / static_cast<float>(p.second) : 0,
                                 z * ((max.z - min.z) / zGrid) + min.z);
        }
    }

    return outputs;
}

void RenderWindow::setupPlainShader(int shaderIndex)
{
    mMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );
}

void RenderWindow::setupTextureShader(int shaderIndex)
{
    mMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );
    mTextureUniform = glGetUniformLocation(mShaderProgram[shaderIndex]->getProgram(), "textureSampler");
}
float RenderWindow::getTerrainHeight(gsl::Vector3D inLocation)
{
    Triangle* currentTriangle = getBallToPlaneTriangle(gsl::Vector3D(inLocation.getX(), 0.f, inLocation.getZ()));
    if (currentTriangle != nullptr)
    {
        gsl::Vector3D pointCoords = gsl::barCoord(
                  gsl::Vector3D(inLocation.getX(), 0.f, inLocation.getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getZ())
                );


        tempTerrainHeight = 1.f + ((pointCoords.getX() * mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getY()) +
                            (pointCoords.getY() * mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getY()) +
                            (pointCoords.getZ() * mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getY()));

        tempTerrainHeight = tempTerrainHeight * terrainHeightScale;
    }
    else
    {
        //qDebug() << "oof";
    }
    return tempTerrainHeight;
}

void RenderWindow::moveBallAlongSpline(BSplineCurve *curve, VisualObject *objectToMove)
{
    float deltaTime = mTimeStart.nsecsElapsed() / 10000000.f;
    deltaTime = deltaTime / 5.f;

    if (goingForward)
    {
        timeCounter += deltaTime;
    }
    else if (!goingForward)
    {
        timeCounter -= deltaTime;
    }

    if (timeCounter > 1.f)
    {
        goingForward = false;
        curve->eventEndOfSpline();
    }
    else if (timeCounter < 0.f)
    {
        goingForward = true;
        curve->eventEndOfSpline();
    }



    Triangle* currentTriangle = getBallToPlaneTriangle(gsl::Vector3D(curve->getCurrentSplineLocation(timeCounter).getX(), 0.f, curve->getCurrentSplineLocation(timeCounter).getZ()));
    if (currentTriangle != nullptr)
    {
        gsl::Vector3D pointCoords = gsl::barCoord(
                    gsl::Vector3D(curve->getCurrentSplineLocation(timeCounter).getX(), 0.f, curve->getCurrentSplineLocation(timeCounter).getZ())
                    , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getZ())
                );


        heightAtSpline = ((pointCoords.getX() * mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getY()) +
                (pointCoords.getY() * mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getY()) +
                (pointCoords.getZ() * mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getY()));

        heightAtSpline = (heightAtSpline * terrainHeightScale) + 1.f;
    }
    else
    {
        //qDebug() << "oof";
    }


    objectToMove->mMatrix.setPosition(curve->getCurrentSplineLocation(timeCounter).getX(), heightAtSpline, curve->getCurrentSplineLocation(timeCounter).getZ());


    //qDebug() << timeCounter;
}

void RenderWindow::inputMoveBall(ballDirection direction)
{
    //qDebug() << mVisualObjects[3]->mMatrix.getPosition().getY();
    if (direction == ballDirection::UP)
    {
        mVisualObjects[3]->mMatrix.translate(gsl::Vector3D(0.2f, 0.f, 0.f));
    }
    if (direction == ballDirection::DOWN)
    {
        mVisualObjects[3]->mMatrix.translate(gsl::Vector3D(-0.2f, 0.f, 0.f));
    }
    if (direction == ballDirection::RIGHT)
    {
        mVisualObjects[3]->mMatrix.translate(gsl::Vector3D(0.f, 0.f, -0.2f));
    }
    if (direction == ballDirection::LEFT)
    {
        mVisualObjects[3]->mMatrix.translate(gsl::Vector3D(0.f, 0.f, 0.2f));
    }

    Triangle* currentTriangle = getBallToPlaneTriangle(gsl::Vector3D(mVisualObjects[3]->mMatrix.getPosition().getX(), 0.f, mVisualObjects[3]->mMatrix.getPosition().getZ()));
    if (currentTriangle != nullptr)
    {
        playerCoords = gsl::barCoord(
                    gsl::Vector3D(mVisualObjects[3]->mMatrix.getPosition().getX(), 0.f, mVisualObjects[3]->mMatrix.getPosition().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getZ())
                , gsl::Vector3D(mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getX(), 0.f, mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getZ())
                );

        playerHeight = ((playerCoords.getX() * mTerrainVertices.at(currentTriangle->index[0]).get_xyz().getY()) +
                (playerCoords.getY() * mTerrainVertices.at(currentTriangle->index[1]).get_xyz().getY()) +
                (playerCoords.getZ() * mTerrainVertices.at(currentTriangle->index[2]).get_xyz().getY()));

        playerHeight = playerHeight * terrainHeightScale;
        //qDebug() << playerCoords;
    }

    mVisualObjects[3]->mMatrix.setPosition(mVisualObjects[3]->mMatrix.getPosition().getX(),
            playerHeight + 1.f,
            mVisualObjects[3]->mMatrix.getPosition().getZ());

    //qDebug() << mVisualObjects[3]->mMatrix.getPosition();
}

void RenderWindow::moveBall(float deltaTime)
{

    auto& ball = *mVisualObjects[2];
    ball.velocity += ball.mAcceleration * deltaTime;
    //std::cout << ball.mAcceleration << endl;
    auto pos = ball.mMatrix.getPosition() + ball.velocity * deltaTime;

    auto hitResults = isColliding(&ball, 1.f);
    if (hitResults.first)
    {
        ball.velocity -= gsl::project(ball.velocity, hitResults.second);
        //pos = ball.mMatrix.getPosition() + ball.velocity * deltaTime +
        pos = ball.mMatrix.getPosition() + ball.velocity * deltaTime + 0.5f * ball.mAcceleration * deltaTime * deltaTime +
                (gsl::project(-ball.mAcceleration, hitResults.second) + ball.mAcceleration) * std::pow(deltaTime, 2) * 0.5f;
    }
    else
    {
        //pos = ball.mMatrix.getPosition() + ball.velocity * deltaTime;
        pos = ball.mMatrix.getPosition() + ball.velocity * deltaTime + 0.5f * ball.mAcceleration * deltaTime * deltaTime;
    }

    ball.mMatrix.setPosition(pos.x, pos.y, pos.z);
    // std::cout << "velocity: " << ball.velocity << std::endl;
}


void RenderWindow::checkIfPlayerIsCloseToTrophy()
{
    for (unsigned int i = 0; i < trophyPositions.size(); ++i)
    {
        if ((
              gsl::Vector3D(mVisualObjects[3]->mMatrix.getPosition().getX(), 0.f, mVisualObjects[3]->mMatrix.getPosition().getZ())
            - gsl::Vector3D(trophyPositions[i].getX(), 0.f, trophyPositions[i].getZ())).length()
            < 3.f)
        {
            //qDebug() << "Trophy " << i << " is overlapping";
            if (!(static_cast<BSplineCurve*>(mVisualObjects[4])->getIsTrophyPickedUp(i)))
            {
                static_cast<BSplineCurve*>(mVisualObjects[4])->setTrophyStatus(true, i);
                //updateSplineHeight(static_cast<BSplineCurve*>(mVisualObjects[4]));
                qDebug() << "Trophy " << i << " picked up";

            }
        }
    }

//    qDebug() << (mVisualObjects[3]->mMatrix.getPosition() - trophyPositions[1]).length();

//    for (auto trophyPos : trophyPositions)
//    {
//        qDebug() << trophyPos;
//    }

//    qDebug() << trophyPositions.size();
}
std::pair<bool, gsl::vec3> RenderWindow::isColliding(VisualObject* ball, float ballRadius)
{
    auto* tri = getBallToPlaneTriangle(ball->mMatrix.getPosition());
    if (tri != nullptr)
    {
        gsl::vec3 normal = (mTerrainVertices.at(tri->index[1]).get_xyz() - mTerrainVertices.at(tri->index[0]).get_xyz())
                ^ (mTerrainVertices.at(tri->index[2]).get_xyz() - mTerrainVertices.at(tri->index[0]).get_xyz());
        normal.normalize();
        // std::cout << "Normal is: " << normal << std::endl;

        auto toBall = gsl::project(ball->mMatrix.getPosition() - mTerrainVertices.at(tri->index[0]).get_xyz(), normal);

        if (toBall.length() < ballRadius && 0 < toBall * normal)
        {
            // Calculate force
            return {true, normal};
        }
    }
    // ball->mAcceleration = gsl::vec3{0.f, -9.81, 0.f};
    return {false, {0, 0, 0}};
}

Triangle *RenderWindow::getBallToPlaneTriangle(gsl::vec3 ballPos)
{
    if (mTerrainTriangles.empty())
        return nullptr;

    unsigned int index = 0;
    auto* t = &mTerrainTriangles.at(index);
    std::array<gsl::vec3, 3> triangle;
    for (unsigned int i{0}; i < 3; ++i)
    {
        triangle.at(i) = mTerrainVertices.at(t->index[i]).get_xyz();
        triangle.at(i).y = 0.f;
    }

    gsl::vec3 bCoords = gsl::barCoord(gsl::vec3{ballPos.x, 0.f, ballPos.z}, triangle.at(0), triangle.at(1), triangle.at(2));
    unsigned int lastIndex = std::numeric_limits<unsigned int>::max();
    while (!(0 <= bCoords.x && 0 <= bCoords.y && 0 <= bCoords.z))
    {
        unsigned int lowestIndex{0};
        lowestIndex = (bCoords.y < bCoords.x) ? 1 : lowestIndex;
        lowestIndex = (bCoords.z < bCoords.x) ? 2 : lowestIndex;

        if (t->neighbour[lowestIndex] < 0)
            return nullptr;

        unsigned int nextIndex = t->neighbour[lowestIndex];
        if (lastIndex == nextIndex)
            return nullptr;

        lastIndex = index;
        index = nextIndex;

        t = &mTerrainTriangles.at(index);
        for (unsigned int i{0}; i < 3; ++i)
        {
            triangle.at(i) = mTerrainVertices.at(t->index[i]).get_xyz();
            triangle.at(i).y = 0.f;
        }

        bCoords = gsl::barCoord(gsl::vec3{ballPos.x, 0.f, ballPos.z}, triangle.at(0), triangle.at(1), triangle.at(2));
    }

    return t;
}

//This function is called from Qt when window is exposed (shown)
//and when it is resized
//exposeEvent is a overridden function from QWindow that we inherit from
void RenderWindow::exposeEvent(QExposeEvent *)
{
    if (!mInitialized)
        init();

    //This is just to support modern screens with "double" pixels
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, static_cast<GLint>(width() * retinaScale), static_cast<GLint>(height() * retinaScale));

    //If the window actually is exposed to the screen we start the main loop
    //isExposed() is a function in QWindow
    if (isExposed())
    {
        //This timer runs the actual MainLoop
        //16 means 16ms = 60 Frames pr second (should be 16.6666666 to be exact..)
        mRenderTimer->start(16);
        mTimeStart.start();
    }
    mAspectratio = static_cast<float>(width()) / height();
    //    qDebug() << mAspectratio;
    mCurrentCamera->mProjectionMatrix.perspective(45.f, mAspectratio, 1.f, 100.f);
    //    qDebug() << mCamera.mProjectionMatrix;
}

//Simple way to turn on/off wireframe mode
//Not totally accurate, but draws the objects with
//lines instead of filled polygons
void RenderWindow::toggleWireframe()
{
    mWireframe = !mWireframe;
    if (mWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    //turn on wireframe mode
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    //turn off wireframe mode
        glEnable(GL_CULL_FACE);
    }
}

//The way this is set up is that we start the clock before doing the draw call,
//and check the time right after it is finished (done in the render function)
//This will approximate what framerate we COULD have.
//The actual frame rate on your monitor is limited by the vsync and is probably 60Hz
void RenderWindow::calculateFramerate()
{
    long long nsecElapsed = mTimeStart.nsecsElapsed();
    static int frameCount{0};                       //counting actual frames for a quick "timer" for the statusbar

    if (mMainWindow)    //if no mainWindow, something is really wrong...
    {
        ++frameCount;
        if (frameCount > 30) //once pr 30 frames = update the message twice pr second (on a 60Hz monitor)
        {
            //showing some statistics in status bar
            mMainWindow->statusBar()->showMessage(" Time pr FrameDraw: " +
                                                  QString::number(nsecElapsed/1000000., 'g', 4) + " ms  |  " +
                                                  "FPS (approximated): " + QString::number(1E9 / nsecElapsed, 'g', 7));
            frameCount = 0;     //reset to show a new message in 60 frames
        }
    }
}


/// Uses QOpenGLDebugLogger if this is present
/// Reverts to glGetError() if not
void RenderWindow::checkForGLerrors()
{
    if(mOpenGLDebugLogger)
    {
        const QList<QOpenGLDebugMessage> messages = mOpenGLDebugLogger->loggedMessages();
        for (const QOpenGLDebugMessage &message : messages)
            qDebug() << message;
    }
    else
    {
        GLenum err = GL_NO_ERROR;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            qDebug() << "glGetError returns " << err;
        }
    }
}

/// Tries to start the extended OpenGL debugger that comes with Qt
void RenderWindow::startOpenGLDebugger()
{
    QOpenGLContext * temp = this->context();
    if (temp)
    {
        QSurfaceFormat format = temp->format();
        if (! format.testOption(QSurfaceFormat::DebugContext))
            qDebug() << "This system can not use QOpenGLDebugLogger, so we revert to glGetError()";

        if(temp->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
        {
            qDebug() << "System can log OpenGL errors!";
            mOpenGLDebugLogger = new QOpenGLDebugLogger(this);
            if (mOpenGLDebugLogger->initialize()) // initializes in the current context
                qDebug() << "Started OpenGL debug logger!";
        }

        if(mOpenGLDebugLogger)
            mOpenGLDebugLogger->disableMessages(QOpenGLDebugMessage::APISource, QOpenGLDebugMessage::OtherType, QOpenGLDebugMessage::NotificationSeverity);
    }
}

void RenderWindow::setCameraSpeed(float value)
{
    mCameraSpeed += value;

    //Keep within min and max values
    if(mCameraSpeed < 0.01f)
        mCameraSpeed = 0.01f;
    if (mCameraSpeed > 0.3f)
        mCameraSpeed = 0.3f;
}

bool RenderWindow::readTerrainData(std::string file)
{
    std::cout << "read ";
    std::ifstream ifs{file};
    if (ifs)
    {
        unsigned int n{0};
        ifs >> n;
        mTerrainVertices.reserve(n);
        std::cout << "inserted terrainvertices";
        for (unsigned int i{0}; i < n; ++i)
        {
            Vertex v;
            ifs >> v;
            mTerrainVertices.push_back(v);
        }

        ifs >> n;
        mTerrainTriangles.reserve(n);
        std::cout << "inserted triangles";
        for (unsigned int i{0}; i < n; ++i)
        {
            Triangle t;
            ifs >> t;
            mTerrainTriangles.push_back(t);
            //std::cout << t;
        }

        return true;
    }
    else{
        std::cout << "no ifs";
        return false;}
}

void RenderWindow::initTerrain()
{
    //readTerrainData("../GSOpenGL2019/SecondTriangle.txt");

        constructTerrain();


    std::cout << "Triangle count: " << mTerrainTriangles.size() << std::endl;

    glGenVertexArrays(1, &mTerrainVAO);
    glBindVertexArray(mTerrainVAO);

    GLuint terrainVBO, terrainEBO;
    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, mTerrainVertices.size() * sizeof(Vertex), mTerrainVertices.data(), GL_STATIC_DRAW);


    unsigned int *data = new unsigned int[mTerrainTriangles.size() * 3];
    for (unsigned int i{0}; i < mTerrainTriangles.size(); ++i)
        for (unsigned int j{0}; j < 3; ++j)
            data[i * 3 + j] = mTerrainTriangles.at(i).index[j];

    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mTerrainTriangles.size() * 3 * sizeof(unsigned int), data, GL_STATIC_DRAW);

    delete[] data;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // glPointSize(10.f);
}

void RenderWindow::constructTerrain()
{
    gsl::LASLoader loader{"../VSIM_Folder3/Mountain.las"};

    mTerrainModMat.setToIdentity();
    mTerrainModMat.scale(1.f,terrainHeightScale,1.f);
    bool flipY = true;

    gsl::Vector3D min{};
    gsl::Vector3D max{};
    std::vector<gsl::Vector3D> terrainPoints;

    terrainPoints.reserve(loader.pointCount());
    for (auto it = loader.begin(); it != loader.end(); it = it + 10)
    {
        terrainPoints.emplace_back(it->xNorm(), (flipY) ? 1.0 - it->zNorm() : it->zNorm(), it->yNorm());

        min.x = (terrainPoints.back().x < min.x) ? terrainPoints.back().x : min.x;
        min.y = (terrainPoints.back().y < min.y) ? terrainPoints.back().y : min.y;
        min.z = (terrainPoints.back().z < min.z) ? terrainPoints.back().z : min.z;

        max.x = (terrainPoints.back().x > max.x) ? terrainPoints.back().x : max.x;
        max.y = (terrainPoints.back().y > max.y) ? terrainPoints.back().y : max.y;
        max.z = (terrainPoints.back().z > max.z) ? terrainPoints.back().z : max.z;
    }

    int xGridSize{50}, zGridSize{50};
    terrainPoints = mapToGrid(terrainPoints, xGridSize, zGridSize, min, max);
    terrainPoints.shrink_to_fit();



    mTerrainVertices.reserve(terrainPoints.size());
    std::transform(terrainPoints.begin(), terrainPoints.end(), std::back_inserter(mTerrainVertices), [](const gsl::Vector3D& point){
        return Vertex{(point - 0.5f) * 40.f, {point.x, 0.33f, point.z}, {0, 0}};
    });

    std::cout << "Point count: " << mTerrainVertices.size() << std::endl;

    // Create indices
    mTerrainTriangles.reserve((xGridSize - 1) * (zGridSize - 1) * 2);
    for (unsigned int z{0}, i{0}; z < zGridSize - 1; ++z, ++i)
    {
        for (unsigned int x{0}; x < xGridSize - 1; ++x, ++i)
        {
            mTerrainTriangles.push_back({{i, i + xGridSize, i + 1},
                                        {
                                            static_cast<int>(mTerrainTriangles.size()) + 1,
                                            (z != 0) ? static_cast<int>(static_cast<int>(mTerrainTriangles.size()) - (xGridSize - 1) * 2 + 1) : -1,
                                            (x != 0) ? static_cast<int>(mTerrainTriangles.size()) - 1 : -1
                                        }
                                        });

//            std::cout << "Added a triangle with index: " << mTerrainTriangles.back().index[0] << ", " << mTerrainTriangles.back().index[1]
//                      << ", " << mTerrainTriangles.back().index[2] << " and neighbours: " << mTerrainTriangles.back().neighbour[0]
//                      << ", " << mTerrainTriangles.back().neighbour[1] << ", " << mTerrainTriangles.back().neighbour[2] << std::endl;

            mTerrainTriangles.push_back({{i + 1, i + xGridSize, i + 1 + xGridSize} ,
                                        {
                                            (z < zGridSize - 2) ? static_cast<int>(static_cast<int>(mTerrainTriangles.size()) + (zGridSize - 1) * 2 - 1) : -1,
                                            (x < xGridSize - 2) ? static_cast<int>(mTerrainTriangles.size() + 1) : -1,
                                            static_cast<int>(mTerrainTriangles.size()) - 1
                                        }
                                        });

//            std::cout << "Added a triangle with index: " << mTerrainTriangles.back().index[0] << ", " << mTerrainTriangles.back().index[1]
//                      << ", " << mTerrainTriangles.back().index[2] << " and neighbours: " << mTerrainTriangles.back().neighbour[0]
//                      << ", " << mTerrainTriangles.back().neighbour[1] << ", " << mTerrainTriangles.back().neighbour[2] << std::endl;
        }
    }

}


void RenderWindow::handleInput()
{
    //Camera
    mCurrentCamera->setSpeed(0.f);  //cancel last frame movement
    if(mInput.RMB)
    {
        if(mInput.W)
            mCurrentCamera->setSpeed(-mCameraSpeed);
        if(mInput.S)
            mCurrentCamera->setSpeed(mCameraSpeed);
        if(mInput.D)
            mCurrentCamera->moveRight(mCameraSpeed);
        if(mInput.A)
            mCurrentCamera->moveRight(-mCameraSpeed);
        if(mInput.Q)
            mCurrentCamera->updateHeigth(mCameraSpeed);
        if(mInput.E)
            mCurrentCamera->updateHeigth(-mCameraSpeed);
    }




    if (mInput.W && !mInput.RMB)
    {
        inputMoveBall(ballDirection::UP);
    }
    if (mInput.S && !mInput.RMB)
    {
        inputMoveBall(ballDirection::DOWN);
    }
    if (mInput.D && !mInput.RMB)
    {
        inputMoveBall(ballDirection::RIGHT);
    }
    if (mInput.A && !mInput.RMB)
    {
        inputMoveBall(ballDirection::LEFT);
    }

    if (mInput.LMB)
    {
        mVisualObjects[2]->mAcceleration = gsl::vec3{0.f, -9.81f, 0.f};
        mVisualObjects[2]->velocity = gsl::vec3{0, 0, 0};
        mVisualObjects[2]->mMatrix.setToIdentity();
        mVisualObjects[2]->mMatrix.setPosition(0.f, 10.f, 0.f);
    }
        //mSimulationTime -= deltaTime;
}

void RenderWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) //Shuts down whole program
    {
        mMainWindow->close();
    }

    //    You get the keyboard input like this
    if(event->key() == Qt::Key_W)
    {
        mInput.W = true;
    }
    if(event->key() == Qt::Key_S)
    {
        mInput.S = true;
    }
    if(event->key() == Qt::Key_D)
    {
        mInput.D = true;
    }
    if(event->key() == Qt::Key_A)
    {
        mInput.A = true;
    }
    if(event->key() == Qt::Key_Q)
    {
        mInput.Q = true;
    }
    if(event->key() == Qt::Key_E)
    {
        mInput.E = true;
    }
    if(event->key() == Qt::Key_Z)
    {
    }
    if(event->key() == Qt::Key_X)
    {
    }
    if(event->key() == Qt::Key_Up)
    {
        mInput.UP = true;
    }
    if(event->key() == Qt::Key_Down)
    {
        mInput.DOWN = true;
    }
    if(event->key() == Qt::Key_Left)
    {
        mInput.LEFT = true;
    }
    if(event->key() == Qt::Key_Right)
    {
        mInput.RIGHT = true;
    }
    if(event->key() == Qt::Key_U)
    {
    }
    if(event->key() == Qt::Key_O)
    {
    }
}

void RenderWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_W)
    {
        mInput.W = false;
    }
    if(event->key() == Qt::Key_S)
    {
        mInput.S = false;
    }
    if(event->key() == Qt::Key_D)
    {
        mInput.D = false;
    }
    if(event->key() == Qt::Key_A)
    {
        mInput.A = false;
    }
    if(event->key() == Qt::Key_Q)
    {
        mInput.Q = false;
    }
    if(event->key() == Qt::Key_E)
    {
        mInput.E = false;
    }
    if(event->key() == Qt::Key_Z)
    {
    }
    if(event->key() == Qt::Key_X)
    {
    }
    if(event->key() == Qt::Key_Up)
    {
        mInput.UP = false;
    }
    if(event->key() == Qt::Key_Down)
    {
        mInput.DOWN = false;
    }
    if(event->key() == Qt::Key_Left)
    {
        mInput.LEFT = false;
    }
    if(event->key() == Qt::Key_Right)
    {
        mInput.RIGHT = false;
    }
    if(event->key() == Qt::Key_U)
    {
    }
    if(event->key() == Qt::Key_O)
    {
    }
}

void RenderWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        mInput.RMB = true;
    if (event->button() == Qt::LeftButton)
        mInput.LMB = true;
    if (event->button() == Qt::MiddleButton)
        mInput.MMB = true;
}

void RenderWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        mInput.RMB = false;
    if (event->button() == Qt::LeftButton)
        mInput.LMB = false;
    if (event->button() == Qt::MiddleButton)
        mInput.MMB = false;
}

void RenderWindow::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

    //if RMB, change the speed of the camera
    if (mInput.RMB)
    {
        if (numDegrees.y() < 1)
            setCameraSpeed(0.001f);
        if (numDegrees.y() > 1)
            setCameraSpeed(-0.001f);
    }
    event->accept();
}

void RenderWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (mInput.RMB)
    {
        //Using mMouseXYlast as deltaXY so we don't need extra variables
        mMouseXlast = event->pos().x() - mMouseXlast;
        mMouseYlast = event->pos().y() - mMouseYlast;

        if (mMouseXlast != 0)
            mCurrentCamera->yaw(mCameraRotateSpeed * mMouseXlast);
        if (mMouseYlast != 0)
            mCurrentCamera->pitch(mCameraRotateSpeed * mMouseYlast);
    }
    mMouseXlast = event->pos().x();
    mMouseYlast = event->pos().y();
}
