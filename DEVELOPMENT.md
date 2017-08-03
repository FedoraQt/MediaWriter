# Development

During development you might want to use a development build of isomd5sum or
iso9660io.

## Using development version of dependencies

```
# TODO(squimrel): Use https://github.com/rhinstaller/isomd5sum master branch
# once PR is merged.
git clone -b cooking https://github.com/squimrel/isomd5sum
cd isomd5sum/
# Uses fedpkg to build the package.
sh scripts/package.sh HEAD

# If the spec file was updated in the development version either a new version
# has to be released or the previous spec file should be deleted manually. If
# it's deleted the changelog will be lost.
rm package/isomd5sum/isomd5sum.spec
sh scripts/package.sh HEAD

sudo dnf install package/isomd5sum/x86_64/isomd5sum-devel-*.rpm
cd ..

git clone https://github.com/squimrel/iso9660io
cd iso9660io/
# Uses fedpkg to build the package.
sh scripts/package.sh HEAD
sudo dnf install package/iso9660io/x86_64/iso9660io-*.rpm
cd ..
```
