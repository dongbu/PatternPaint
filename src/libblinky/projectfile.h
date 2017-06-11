#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include "libblinkyglobal.h"

#include "patterncollectionmodel.h"
#include "patterncollection.h"
#include "fixture.h"
#include "scene.h"


#define PROJECT_HEADER     "PatternPaint Project"   /// Header of PatternPaint Project
#define PROJECT_FORMAT_VERSION     1.0              /// Version of PatternPaint Project format

// The project file consists of the following objects:
// PROJECT_HEADER
// PROJECT_FORMAT_VERSION
// sceneConfiguration
// patternCollection


class LIBBLINKY_EXPORT ProjectFile
{
public:
    ProjectFile();

    bool save(QString filename, Scene *scene, PatternCollection* patternCollection);

    bool open(QString filename, Scene *scene, PatternCollection* patternCollection);

    void writeHeader(QDataStream& stream, float version);

    float readHeader(QDataStream& stream);

};

#endif // PROJECTFILE_H