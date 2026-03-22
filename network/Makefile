##################################################################
#  Copyright(C) 2023 CANLAB Inc.	             				 #
#  Garnet Kim <garnet@canlab.co.kr> - 2023.01.          		 #
#  Modify       : (c) 2023 CANLAB Inc.	          				 #		
#  File Name    : Makefile.                                      #
##################################################################
sinclude ../Rules.make

OBJS = CLApp_NetworkServer.o \
	CLApp_NetworkSocket.o \
	CLApp_NetMultiThread.o 
	
BASE_LIB = libnetwork.a

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
