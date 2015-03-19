#ifndef STATISTICSEXTENSIONS_H
#define STATISTICSEXTENSIONS_H

#include <QStringList>
#include <QMap>

#if _WIN32 && !__MINGW32__
#define _USE_MATH_DEFINES 1
#include "math.h"
#else
#include <cmath>
#endif

typedef QMap<int,QColor> ColorMap;

class ColorRange {
public:
    ColorRange() {}
    ColorRange(QStringList row) {
        rangeMin = row[2].toInt();
        unsigned char minColorR = row[4].toInt();
        unsigned char minColorG = row[6].toInt();
        unsigned char minColorB = row[8].toInt();
        unsigned char minColorA = row[10].toInt();
        minColor = QColor( minColorR, minColorG, minColorB, minColorA );

        rangeMax = row[3].toInt();
        unsigned char maxColorR = row[5].toInt();
        unsigned char maxColorG = row[6].toInt();
        unsigned char maxColorB = row[9].toInt();
        unsigned char maxColorA = row[11].toInt();
        minColor = QColor( maxColorR, maxColorG, maxColorB, maxColorA );
    }
    virtual ~ColorRange() {}

    virtual QColor getColor(float value) {
        // clamp the value to [min max]
        if (value > rangeMax) value = (float)rangeMax;
        if (value < rangeMin) value = (float)rangeMin;

        unsigned char retR = minColor.red() + (unsigned char)( floor((float)value / (float)(rangeMax-rangeMin) * (float)(maxColor.red()-minColor.red()) + 0.5f) );
        unsigned char retG = minColor.green() + (unsigned char)( floor((float)value / (float)(rangeMax-rangeMin) * (float)(maxColor.green()-minColor.green()) + 0.5f) );
        unsigned char retB = minColor.blue() + (unsigned char)( floor((float)value / (float)(rangeMax-rangeMin) * (float)(maxColor.blue()-minColor.blue()) + 0.5f) );
        unsigned char retA = minColor.alpha() + (unsigned char)( floor((float)value / (float)(rangeMax-rangeMin) * (float)(maxColor.alpha()-minColor.alpha()) + 0.5f) );

        return QColor(retR, retG, retB, retA);
    }

    int rangeMin, rangeMax;
    QColor minColor;
    QColor maxColor;
};

enum defaultColormaps_t {
    jetColormap = 0,
    heatColormap,
    hsvColormap,
    hotColormap,
    coolColormap,
    springColormap,
    summerColormap,
    autumnColormap,
    winterColormap,
    grayColormap,
    boneColormap,
    copperColormap,
    pinkColormap,
    linesColormap
};

class DefaultColorRange : public ColorRange
{
public:
    DefaultColorRange(QStringList &row)
    {
        rangeMin = row[2].toInt();
        rangeMax = row[3].toInt();
        QString str = row[4];
        if (str == "jet")
            type = jetColormap;
        else if (str == "heat")
            type = heatColormap;
        else if (str == "hsv")
            type = hsvColormap;
        else if (str == "hot")
            type = hotColormap;
        else if (str == "cool")
            type = coolColormap;
        else if (str == "spring")
            type = springColormap;
        else if (str == "summer")
            type = summerColormap;
        else if (str == "autumn")
            type = autumnColormap;
        else if (str == "winter")
            type = winterColormap;
        else if (str == "gray")
            type = grayColormap;
        else if (str == "bone")
            type = boneColormap;
        else if (str == "copper")
            type = copperColormap;
        else if (str == "pink")
            type = pinkColormap;
        else if (str == "lines")
            type = linesColormap;
    }

    virtual QColor getColor(float value)
    {
        float span = (float)(rangeMax-rangeMin),
                val = (float)value,
                x = val / span,
                r=1,g=1,b=1,a=1;
        float F,N,K;
        int I;

        // clamp the value to [min max]
        if (value > rangeMax) value = (float)rangeMax;
        if (value < rangeMin) value = (float)rangeMin;

        switch (type) {
        case jetColormap:
            if ((x >= 3.0/8.0) && (x < 5.0/8.0)) r = (4.0f * x - 3.0f/2.0f); else
                if ((x >= 5.0/8.0) && (x < 7.0/8.0)) r = 1.0; else
                if (x >= 7.0/8.0) r = (-4.0f * x + 9.0f/2.0f); else
                    r = 0.0f;
            if ((x >= 1.0/8.0) && (x < 3.0/8.0)) g = (4.0f * x - 1.0f/2.0f); else
                if ((x >= 3.0/8.0) && (x < 5.0/8.0)) g = 1.0; else
                if ((x >= 5.0/8.0) && (x < 7.0/8.0)) g = (-4.0f * x + 7.0f/2.0f); else
                    g = 0.0f;
            if (x < 1.0/8.0) b = (4.0f * x + 1.0f/2.0f); else
                if ((x >= 1.0/8.0) && (x < 3.0/8.0)) b = 1.0f; else
                if ((x >= 3.0/8.0) & (x < 5.0/8.0)) b = (-4.0f * x + 5.0f/2.0f); else
                    b = 0.0f;
            break;
        case heatColormap:
            r = 1;
            g = 0;
            b = 0;
            a = x;
            break;
        case hsvColormap:  // h = x, s = 1, v = 1
            if (x >= 1.0) x = 0.0;
            x = x * 6.0f;
            I = (int) x;   /* should be in the range 0..5 */
            F = x - I;     /* fractional part */

            N = (1.0f - 1.0f * F);
            K = (1.0f - 1.0f * (1 - F));

            if (I == 0) { r = 1; g = K; b = 0; }
            if (I == 1) { r = N; g = 1.0; b = 0; }
            if (I == 2) { r = 0; g = 1.0; b = K; }
            if (I == 3) { r = 0; g = N; b = 1.0; }
            if (I == 4) { r = K; g = 0; b = 1.0; }
            if (I == 5) { r = 1.0; g = 0; b = N; }
            break;
        case hotColormap:
            if (x < 2.0/5.0) r = (5.0f/2.0f * x); else r = 1.0f;
            if ((x >= 2.0/5.0) && (x < 4.0/5.0)) g = (5.0f/2.0f * x - 1); else if (x >= 4.0/5.0) g = 1.0f; else g = 0.0f;
            if (x >= 4.0/5.0) b = (5.0f*x - 4.0f); else b = 0.0f;
            break;
        case coolColormap:
            r = x;
            g = 1.0f - x;
            b = 1.0;
            break;
        case springColormap:
            r = 1.0f;
            g = x;
            b = 1.0f - x;
            break;
        case summerColormap:
            r = x;
            g = 0.5f + x / 2.0f;
            b = 0.4f;
            break;
        case autumnColormap:
            r = 1.0;
            g = x;
            b = 0.0;
            break;
        case winterColormap:
            r = 0;
            g = x;
            b = 1.0f - x / 2.0f;
            break;
        case grayColormap:
            r = x;
            g = x;
            b = x;
            break;
        case boneColormap:
            if (x < 3.0/4.0) r = (7.0f/8.0f * x); else if (x >= 3.0/4.0) r = (11.0f/8.0f * x - 3.0f/8.0f);
            if (x < 3.0/8.0) g = (7.0f/8.0f * x); else
                if ((x >= 3.0/8.0) && (x < 3.0/4.0)) g = (29.0f/24.0f * x - 1.0f/8.0f); else
                if (x >= 3.0/4.0) g = (7.0f/8.0f * x + 1.0f/8.0f);
            if (x < 3.0/8.0) b = (29.0f/24.0f * x); else if (x >= 3.0/8.0) b = (7.0f/8.0f * x + 1.0f/8.0f);
            break;
        case copperColormap:
            if (x < 4.0/5.0) r = (5.0f/4.0f * x); else r = 1.0f;
            g = 4.0f/5.0f * x;
            b = 1.0f/2.0f * x;
            break;
        case pinkColormap:
            if (x < 3.0/8.0) r = (14.0f/9.0f * x); else
                if (x >= 3.0/8.0) r = (2.0f/3.0f * x + 1.0f/3.0f);
            if (x < 3.0/8.0) g = (2.0f/3.0f * x); else
                if ((x >= 3.0/8.0) && (x < 3.0/4.0)) g = (14.0f/9.0f * x - 1.0f/3.0f); else
                if (x >= 3.0/4.0) g = (2.0f/3.0f * x + 1.0f/3.0f);
            if (x < 3.0/4.0)b = (2.0f/3.0f * x); else
                if (x >= 3.0/4.0) b = (2.0f * x - 1.0f);
            break;
        case linesColormap:
            if (x >= 1.0) x = 0.0f;
            x = x * 7.0f;
            I = (int) x;

            if (I == 0) { r = 0.0; g = 0.0; b = 1.0; }
            if (I == 1) { r = 0.0; g = 0.5; b = 0.0; }
            if (I == 2) { r = 1.0; g = 0.0; b = 0.0; }
            if (I == 3) { r = 0.0; g = 0.75; b = 0.75; }
            if (I == 4) { r = 0.75; g = 0.0; b = 0.75; }
            if (I == 5) { r = 0.75; g = 0.75; b = 0.0; }
            if (I == 6) { r = 0.25; g = 0.25; b = 0.25; }
            break;
        }

        //TODO: proper rounding
        unsigned char retR = (unsigned char)( floor(r * 255.0f + 0.5f) );
        unsigned char retG = (unsigned char)( floor(g * 255.0f + 0.5f) );
        unsigned char retB = (unsigned char)( floor(b * 255.0f + 0.5f) );
        unsigned char retA = (unsigned char)( floor(a * 255.0f + 0.5f) );

        return QColor(retR, retG, retB, retA);
    }

private:
    defaultColormaps_t type;
};

enum visualizationType_t { colorMapType, colorRangeType, vectorType };
class StatisticsType
{
public:
    StatisticsType()
    {
        typeID = -1;
        typeName = "?";
        render = false;
        renderGrid = false;
        alphaFactor = 50;

        colorRange = NULL;

        vectorSampling = 1;
        scaleToBlockSize = false;
    }
    ~StatisticsType()
    {
        if( colorRange == NULL ) { delete colorRange; colorRange = NULL; }
    }

    void readFromRow(QStringList row)
    {
        if( row.count() >= 5 )
        {
            assert( typeID == row[2].toInt() ); // typeID needs to be set already
            typeName = row[3];
            if (row[4] == "map") visualizationType = colorMapType;
            else if (row[4] == "range") visualizationType = colorRangeType;
            else if (row[4] == "vector") visualizationType = vectorType;
        }
    }

    int typeID;
    QString typeName;
    visualizationType_t visualizationType;
    bool scaleToBlockSize;

    // only for vector type
    int vectorSampling;

    // only one of the next should be set, depending on type
    ColorMap colorMap;
    ColorRange* colorRange; // can either be a ColorRange or a DefaultColorRange
    QColor vectorColor;
    QColor gridColor;

    // parameters controlling visualization
    bool    render;
    bool    renderGrid;
    int     alphaFactor;
};

enum statistics_t {
    arrowType = 0,
    blockType
};

struct StatisticsItem {
    statistics_t type;
    QColor color;
    QColor gridColor;
    QRect positionRect;
    float vector[2];
    int rawValues[2];
};

#endif // STATISTICSEXTENSIONS_H

