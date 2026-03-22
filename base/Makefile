sinclude ../Rules.make

OBJS = 	App_Condition.o \
	App_Mutex.o \
	App_Thread.o \
	App_Command.o \
	App_MsgQueue.o

	
BASE_LIB = libbase.a

CFLAGS += -Wno-psabi

$(BASE_LIB) : $(OBJS) 
	$(AR) qcs $@ $(OBJS) 
	@(cp $@ $(LIB_DIR))
	@(echo "Copy $@ to $(LIB_DIR)")
	
.cpp.o : $(OBJS)
	$(CPP) $(CFLAGS) -c $< $(DEBUG_MODE) -D_DEBUG_

clean:
	rm -f $(BASE_LIB) $(OBJS)
	rm -f $(LIB_DIR)/$(BASE_LIB)

dep: $(OBJS)
	$(CPP) -MM $(CFLAGS) $< > .depend

.depend:
	echo > .depend

include .depend
