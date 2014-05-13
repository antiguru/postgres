# contrib/postgres_fdw/Makefile

MODULE_big = sprocket
OBJS = sprocket.o

PG_CPPFLAGS = -I$(libpq_srcdir) --std=c99 -Werror
SHLIB_LINK = $(libpq)
SHLIB_PREREQS = submake-libpq

EXTENSION = sprocket
DATA = sprocket--1.0.sql

REGRESS = sprocket

# the db name is hard-coded in the tests
override USE_MODULE_DB =

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/sprocket
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
