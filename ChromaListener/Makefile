LINK_TARGET = ChromaListenerLinux

OBJS =  \
 razerkraken_driver.o \
 razerkbd_driver.o \
 razermouse_driver.o \
 razermug_driver.o \
 razerfirefly_driver.o \
 razerchromacommon.o \
 razercommon.o \
 ChromaListener.o \
 ChromaAPI.o \
 Chroma.o \
 tinystr.o \
 tinyxmlerror.o \
 tinyxmlparser.o \
 tinyxml.o
 
REBUILDABLES = $(OBJS) $(LINK_TARGET)

all: $(LINK_TARGET)
	echo All done

$(LINK_TARGET) : $(OBJS)
	g++ -g -o $@ $^

%.o : %.cpp
	g++ -std=gnu++11 -g -o $@ -c $<

clean:
	rm -f $(REBUILDABLES)
	echo Clean done

