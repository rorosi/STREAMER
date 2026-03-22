sinclude ../Rules.make

MAIN_EXE = stream_daemon

OBJS = main.o

LIBS += -lbase
LIBS += -ldl

.PHONY: clean $(MAIN_EXE) 

all: $(MAIN_EXE)

$(MAIN_EXE) : $(OBJS)
	$(CPP) -o $@ $(OBJS) -lcontrol $(LIBDIR) $(LIBS) -lrt -lpthread 
	$(STRIP) $@	
	@(cp $@ $(BIN_DIR))
	
.cpp.o : $(OBJS)
	$(CPP) $(CFLAGS) -c $< $(DEBUG_MODE)

clean: 
	rm -f *.o $(MAIN_EXE) 
	rm -f $(BIN_DIR)/$(MAIN_EXE)
	-$(RM) -rf release 

