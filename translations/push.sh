#!/bin/bash

rm -f mediawriter.pot mediawriter.ts
lupdate-qt5 ../mediawriter.pro -ts mediawriter.ts
lconvert-qt5 -of po -o mediawriter.pot mediawriter.ts
zanata-cli push -B
