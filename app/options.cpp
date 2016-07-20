#include "options.h"

Options options;

void Options::parse(QStringList argv) {
    if (argv.contains("--testing"))
        testing = true;
}
