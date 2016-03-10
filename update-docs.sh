#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR

# make sure we aren't on the gh-pages branch
branch_name="$(git symbolic-ref HEAD 2>/dev/null)" ||
branch_name="(unnamed branch)"     # detached HEAD


if [ "$branch_name" == "gh-pages" ]
then 
	exit 1
fi

cd $DIR/build


# Run doxygen to make sure we have the latest
cmake ..
make docs

# copy those to a tempory directory

TMP_DIR="$(mktemp -d)"

echo "Using tempory directory $TMP_DIR"

cp -r $DIR/build/doc $TMP_DIR/doc

# checkout gh-pages
git checkout gh-pages

# copy them over
cp -R $TMP_DIR/doc/html/* $DIR

# delete the tempory directory
rm -r $TMP_DIR

# start push
git add .
git commit -a -m "Update doxygen documentation"
git push
git checkout $branch_name
