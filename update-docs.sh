 #!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR

# make sure we aren't on the gh-pages branch
branch_name="$(git symbolic-ref HEAD 2>/dev/null)" ||
branch_name="(unnamed branch)"     # detached HEAD

if [[ "$branch_name" -eq "gh-pages" ]]
then 
	exit 1
fi


# Run doxygen to make sure we have the latest

mkdir -p $DIR/build

cd $DIR/build

cmake -DBUILD_DOCS=ON ..
make docs

# copy those to a tempory directory

TMP_DIR="$(mktemp -d)"

echo "Using tempory directory $TMP_DIR"

cp $DIR/build/doc $TMP_DIR/doc

# checkout gh-pages
git checkout gh-pages

# remove the old docs, and not hidden files
rm -rf $(ls)

# copy them over
cp {ls $TMP_DIR/doc/html} $DIR/

# start push
git push
