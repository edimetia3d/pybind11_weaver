#!/bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
pybind11_weaver --config  $SCRIPT_DIR/cfg.yaml