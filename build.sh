#/!/bin/bash
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";
cd $SCRIPT_DIR

rm -rf ./dist
python3 -m pip install --upgrade build twine
python3 -m build

if [ "$1" = "upload" ];
  then
    python3 -m twine upload dist/*
  else
    echo Uploading ignored
fi

