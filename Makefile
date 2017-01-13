
#  Copyright 2008 James Peach
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

TARGETS = scnotify rtsig pmns pmdesc pmconv isatty gettime \
	  fammonnitor nsurldownload goodsize base64 \
	  emp array_foreach copyfile cpx aio

CXX := clang++
CFLAGS = -g -Wall
CXXFLAGS = -std=c++0x -g -Wall
COVERAGE := -fprofile-arcs -ftest-coverage

SIMPLE = $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
SIMPLE.CXX = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

# Build a PCP program.
PCPPROG = $(SIMPLE)-lpcp

# Build a program that links with the POSIX realtime libraries.
RTPROG = $(SIMPLE) -lrt

all: $(TARGETS)

sslcert: sslcert.cc
	$(CXX) $(CXXFLAGS) -I/usr/local/ssl/include -L/usr/local/ssl/lib -O0 \
		     -o $@ $^ -lcrypto

pmns: pmns.c
	$(PCPPROG)
pmdesc: pmdesc.c
	$(PCPPROG)
pmconv: pmconv.c
	$(PCPPROG)

rtsig: linux-rtsig.c
	$(RTPROG)

gettime: gettime.c
	$(RTPROG)

isatty: isatty.c
	$(SIMPLE)

fammonitor: fammmonitor.c
	$(SIMPLE) -lfam

goodsize: goodsize.c
	$(SIMPLE)

scnotify: scnotify.c
	$(SIMPLE) -framework SystemConfiguration -framework Foundation

nsurldownload: nsurldownload.m
	$(SIMPLE) -framework Foundation

base64: base64.m
	$(SIMPLE) -DBASE64_TESTMAIN -framework Foundation

emp: emp.m base64.m
	$(SIMPLE) -framework Foundation

array_foreach: array_foreach.cpp
	$(SIMPLE.CXX)

copyfile: copyfile.c
	$(SIMPLE)

cpx: cpx.c
	$(SIMPLE)

aio: aio.c
	$(SIMPLE) -laio

pquota: pquota.cc
	$(SIMPLE.CXX) -lblkid

clean:
	rm -f $(TARGETS) *.o a.out
	rm -rf *.dSYM

.PHONY: all clean
