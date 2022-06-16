# A Modified version from https://github.com/AndrewWalker/ccsyspath
import os
import subprocess
from subprocess import PIPE


def compiler_preprocessor_verbose(compiler, extraflags):
    """Capture the compiler preprocessor stage in verbose mode
    """
    lines = []
    with open(os.devnull, 'r') as devnull:
        cmd = [compiler, '-E']
        cmd += extraflags
        cmd += ['-', '-v']
        p = subprocess.Popen(cmd, stdin=devnull, stdout=PIPE, stderr=PIPE)
        p.wait()
        lines = p.stderr.read()
        lines = lines.splitlines()
        p.stderr.close()
        p.stdout.close()
    return lines


def system_include_paths(compiler, cpp=True):
    extraflags = []
    if cpp:
        extraflags = b'-x c++'.split()
    lines = compiler_preprocessor_verbose(compiler, extraflags)
    lines = [line.strip() for line in lines]

    start = lines.index(b'#include <...> search starts here:')
    end = lines.index(b'End of search list.')

    lines = lines[start + 1:end]
    paths = []
    for line in lines:
        line = line.replace(b'(framework directory)', b'')
        line = line.strip()
        paths.append(line)
    return paths
