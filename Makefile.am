# htmledit - Makefile.am

AM_CFLAGS=-Wall -Wextra -O0 -D_GNU_SOURCE=1

bin_PROGRAMS=csv2html cathtml hexsed csv2anki

csv2anki_SOURCES= \
	src/Utils/fileops.c \
	src/Utils/fileops.h \
	src/Utils/stringops.c \
	src/Utils/stringops.h \
	src/Csv2anki/csv2anki.c \
	src/Csv2anki/gopt.c \
	src/Csv2anki/gopt.h

csv2html_SOURCES= \
	src/Utils/fileops.c \
	src/Utils/fileops.h \
	src/Utils/firstrun.c \
	src/Utils/firstrun.h \
	src/Utils/stringops.c \
	src/Utils/stringops.h \
	src/Csv2html/csv2html.c \
	src/Csv2html/gopt.c \
	src/Csv2html/gopt.h

cathtml_SOURCES= \
	src/Utils/fileops.c \
	src/Utils/fileops.h \
	src/Utils/firstrun.c \
	src/Utils/firstrun.h \
	src/Utils/stringops.c \
	src/Utils/stringops.h \
	src/Cathtml/cathtml.c

hexsed_SOURCES= \
	src/Hexsed/hexsed.c \
	src/Hexsed/gopt.c \
	src/Hexsed/gopt.h \
	src/Utils/fileops.c \
	src/Utils/fileops.h	\
	src/Utils/stringops.c \
	src/Utils/stringops.h

man_MANS= \
	src/Csv2anki/csv2anki.1 \
	src/Csv2html/csv2html.1 \
	src/Cathtml/cathtml.1 \
	src/Hexsed/hexsed.1

# deal with config files
prgdir=$(datadir)/cli_edit

prg_DATA= \
	src/Csv2anki/csv2anki.1 \
	src/Csv2html/csv2html.1 \
	src/Csv2html/csv2html.cfg \
	src/Csv2html/c2hmaster.css \
	src/Csv2html/c2hmaster.xml \
	src/Csv2html/c2hmaster.html \
	src/Cathtml/cathtml.1 \
	src/Cathtml/config \
	src/Hexsed/hexsed.1

EXTRA_DIST= \
	src/Csv2anki/csv2anki.1 \
	src/Csv2html/csv2html.1 \
	src/Csv2html/csv2html.cfg \
	src/Csv2html/c2hmaster.css \
	src/Csv2html/c2hmaster.xml \
	src/Csv2html/c2hmaster.html \
	src/Cathtml/cathtml.1 \
	src/Cathtml/config \
	src/Hexsed/hexsed.1
