# Cryptographic features of AOSC Media Writer

There are two separate checksum validation processes integrated in the AOSC Media Writer.

## SHA256 hash

All AOSC images (except Atomic) have a SHA256 hash assigned when created. This hash is included in the release metadata that's included in AOSC Media Writer and also in the releases.json file that's provided as a part of AOSC Websites and served over HTTPS.

AOSC Media Writer then computes SHA256 hash of the ISO data being downloaded to check if the image is counterfeit or not.

## Integrated MD5 ISO checksum

All AOSC ISO images have an integrated MD5 checksum for integrity purposes.

AOSC Media Writer verifies this checksum right after the image is downloaded and also after the image has been written to a flash drive to verify if it has been written correctly.
