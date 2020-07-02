#!/bin/bash

PROBABLY_TAG=${GITHUB_REF/refs\/tags\//}
REGEXP="[0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*"

grep "$REGEXP" <<< "$PROBABLY_TAG" | sed "s/.*\($REGEXP\).*/\\1/"

exit 0
