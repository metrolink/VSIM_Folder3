#ifndef TROPHY_H
#define TROPHY_H

#include "visualobject.h"

class Trophy : public VisualObject
{
public:
    Trophy();

    void init() override;
    void draw() override;
    void draw(GLint mMatrixUniform);

    void clearAll();

public slots:
    void OnOverlapEvent(VisualObject *object);
};

#endif // TROPHY_H
