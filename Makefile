.EXPORT_ALL_VARIABLES:

include ./Rules.make

SUBS = base \
	control \
	main

BINDIR = bin

all : $(BINDIR)
	@(for c in $(SUBS); do  echo ""; echo "********************* $$c *********************"; \
	($(MAKE) -C $$c || grep -r "error" && exit 0); done); 

clean ::
	@(for c in $(SUBS); do echo ""; echo "********************* $$c *********************"; (cd $$c; $(MAKE) clean) done)
	
dep :
	@(for c in $(SUBS); do echo ""; echo "********************* $$c *********************"; (cd $$c; $(MAKE) dep) done)
	@echo done
	
$(BINDIR) :
	@(if [ ! -d ./$(BINDIR) ]; then \
		mkdir $(BINDIR); \
	fi;)
