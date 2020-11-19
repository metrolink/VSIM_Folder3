#include "trophy.h"
#include "trianglesurface.h"

Trophy::Trophy() {
    Vertex v{};
    //Bottom
    v.set_xyz(-0.2,0,-0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0,0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0,-0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.2,0,0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0,-0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0,0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Top
    v.set_xyz(-0.2,0.4,-0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0.4,0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0.4,-0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.2,0.4,0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0.4,-0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0.4,0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Front
    v.set_xyz(-0.2,0,-0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0.4,-0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0,-0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.2,0.4,-0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0,-0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0.4,-0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Back
    v.set_xyz(0.2,0,0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0.4,0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0,0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(-0.2,0.4,0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0,0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0.4,0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Left
    v.set_xyz(-0.2,0,0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0.4,-0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0,-0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(-0.2,0.4,-0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0,0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.2,0.4,0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Right
    v.set_xyz(0.2,0,-0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0.4,0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0,0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.2,0.4,0.2); v.set_rgb(1,0,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0,-0.2); v.set_rgb(1,0,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.2,0.4,-0.2); v.set_rgb(1,0,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Trophy part
    // Top
    v.set_xyz(-0.1,0.8,-0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.8,0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.8,-0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.1,0.8,0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.8,-0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.8,0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Front
    v.set_xyz(-0.1,0.4,-0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.8,-0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.4,-0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.1,0.8,-0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.4,-0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.8,-0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Back
    v.set_xyz(0.1,0.4,0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.8,0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.4,0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(-0.1,0.8,0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.4,0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.8,0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Left
    v.set_xyz(-0.1,0.4,0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.8,-0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.4,-0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(-0.1,0.8,-0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.4,0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(-0.1,0.8,0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    // Right
    v.set_xyz(0.1,0.4,-0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.8,0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.4,0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);

    v.set_xyz(0.1,0.8,0.1); v.set_rgb(0,1,0); v.set_uv(0,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.4,-0.1); v.set_rgb(0,1,0);  v.set_uv(1,0);
    mVertices.push_back(v);
    v.set_xyz(0.1,0.8,-0.1); v.set_rgb(0,1,0); v.set_uv(1,1);
    mVertices.push_back(v);
}

void Trophy::init() {
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
}

void Trophy::draw()
{
//   glBindVertexArray( mVAO );
//   glDrawArrays(GL_TRIANGLES, 0, mVertices.size());//mVertices.size());
}

void Trophy::draw(GLint mMatrixUniform)
{
    glBindVertexArray( mVAO );
    glUniformMatrix4fv( mMatrixUniform, 1, GL_TRUE, mMatrix.constData());
    glDrawArrays(GL_TRIANGLES, 0, mVertices.size());//mVertices.size());
}

void Trophy::clearAll()
{
    mVertices.clear();
}

//void Trophy::OnOverlapEvent(VisualObject *object)
//{
//    if (object->name() == "Octahedronball")
//    {
//        Pawn *pawn = static_cast<Pawn *>(object);
//        pawn->incTrophies();
//        this->destroy();
//    }
//}
