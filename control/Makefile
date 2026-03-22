sinclude ../Rules.make

OBJS = 	App_StreamManager.o \
		App_RTSPServer.o \
		App_RTPSender.o

	
BASE_LIB = libcontrol.a

CFLAGS += -Wno-psabi

$(BASE_LIB) : $(OBJS) 
	$(AR) qcs $@ $(OBJS) 
	@(cp $@ $(LIB_DIR))
	@(echo "Copy $@ to $(LIB_DIR)")
	
.cpp.o : $(OBJS)
	$(CPP) $(CFLAGS) -c $< 

clean:
	rm -f $(BASE_LIB) $(OBJS)
	rm -f $(LIB_DIR)/$(BASE_LIB)

dep: $(OBJS)
	$(CPP) -MM $(CFLAGS) $< > .depend

.depend:
	echo > .depend

include .depend
