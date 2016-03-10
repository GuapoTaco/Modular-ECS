 #!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR

# Returns "*" if the current git branch is dirty.
function git_dirty {
  [[ $(git diff --shortstat 2> /dev/null | tail -n1) != "" ]] && echo "*"
}


if [ git_dirty == "*" ]; then
	echo "Please commit your changes before running this script";
	exit 4
fi

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

# remove the old docs, and not hidden files
rm -rf $(ls)

# copy them over
cp {ls $TMP_DIR/doc/html} $DIR/

# start push
git push
