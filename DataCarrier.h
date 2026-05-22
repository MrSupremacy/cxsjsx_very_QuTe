#ifndef DATACARRIER_H
#define DATACARRIER_H


struct DataCarrier
{
    DataCarrier(int a, int b, double c, bool d, int e)
        : moveMode(a)
        , difficulty(b)
        , volume(c)
        , timeLimited(d)
        , maxSeconds(e)
    {}

    int moveMode;
    int difficulty;
    double volume;

    bool timeLimited;
    int maxSeconds;
};

struct EndData
{
    EndData(int a, int b)
        : score(a)
        , second(b)
    {}

    int score;
    int second;
};

#endif // DATACARRIER_H
