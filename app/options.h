#ifndef OPTIONS_H
#define OPTIONS_H

#include <QStringList>

struct Options
{
public:
    void parse(QStringList argv);

    bool testing { false };
};

extern Options options;

#endif // OPTIONS_H
