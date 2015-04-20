#!/bin/env python

import yaml
import sys

if len(sys.argv) > 1:
	f = sys.argv[1]
else:
	f = "/dev/stdin"

o = yaml.load(open(f))
print o
