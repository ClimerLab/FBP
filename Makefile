#---------------------------------------------------------------------------------------------------
# Compiler selection
#---------------------------------------------------------------------------------------------------
MPICXX = /usr/local/bin/mpicxx
CXX = g++

#---------------------------------------------------------------------------------------------------
# Directories
#---------------------------------------------------------------------------------------------------

OBJDIR = build
SRCDIR = src

#---------------------------------------------------------------------------------------------------
# Executables
#---------------------------------------------------------------------------------------------------

EXE = FBP

#---------------------------------------------------------------------------------------------------
# Object files
#---------------------------------------------------------------------------------------------------
COMMONOBJ = ConfigParser.o ExprsData.o Parallel.o FBP_Utils.o
CONTROLLEROBJ = Pairs.o Timer.o SolPool.o $(COMMONOBJ)
FBOOBJ = FreqBasedPrunning.o FBP_Controller.o FBP_Worker.o $(CONTROLLEROBJ)

#---------------------------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------------------------

CXXFLAGS = -O3 -Wall -fPIC -fexceptions -DIL_STD -std=c++11 -fno-strict-aliasing

#---------------------------------------------------------------------------------------------------
# Link options and libraries
#---------------------------------------------------------------------------------------------------

OPENMPIT	   = /usr/local/lib/openmpi

CXXLNDIRS      = -L$(OPENMPIT)
CXXLNFLAGS     = -lm -lpthread -ldl

#---------------------------------------------------------------------------------------------------
# -DNDEBUG
all: CXXFLAGS += -DNDEBUG
all: $(EXE)

debug: CXXFLAGS += -g
debug: $(EXE)

FBP: $(addprefix $(OBJDIR)/, FreqBasedPrunning.o)
	$(MPICXX) $(CXXLNDIRS) $(INCLUDES) -o $@ $(addprefix $(OBJDIR)/, $(FBOOBJ)) $(CXXLNFLAGS)
	
$(OBJDIR)/FreqBasedPrunning.o:	$(addprefix $(SRCDIR)/, main.cpp) \
																$(addprefix $(OBJDIR)/, ConfigParser.o Parallel.o FBP_Controller.o FBP_Worker.o)
	$(MPICXX) $(CXXLNDIRS) $(INCLUDES) -c -o $@ $< $(CXXLNFLAGS)

$(OBJDIR)/FBP_Controller.o:	$(addprefix $(SRCDIR)/, FBP_Controller.cpp FBP_Controller.h) \
														$(addprefix $(OBJDIR)/, $(CONTROLLEROBJ))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/FBP_Worker.o:	$(addprefix $(SRCDIR)/, FBP_Worker.cpp FBP_Worker.h) \
												$(addprefix $(OBJDIR)/, FBP_Utils.o) \
												$(addprefix $(OBJDIR)/, $(COMMONOBJ))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/Pairs.o: $(addprefix $(SRCDIR)/, Pairs.cpp Pairs.h)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/SolPool.o:	$(addprefix $(SRCDIR)/, SolPool.cpp SolPool.h) \
											$(addprefix $(OBJDIR)/, ExprsData.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/Pattern.o:	$(addprefix $(SRCDIR)/, Pattern.cpp Pattern.h)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/ExprsData.o:	$(addprefix $(SRCDIR)/, ExprsData.cpp ExprsData.h) \
												$(addprefix $(OBJDIR)/, ConfigParser.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<
	
$(OBJDIR)/ConfigParser.o: $(addprefix $(SRCDIR)/, ConfigParser.cpp ConfigParser.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/FBP_Utils.o: $(addprefix $(SRCDIR)/, FBP_Utils.cpp FBP_Utils.h)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<
	
$(OBJDIR)/Parallel.o: $(addprefix $(SRCDIR)/, Parallel.cpp Parallel.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Timer.o: $(addprefix $(SRCDIR)/, Timer.cpp Timer.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#---------------------------------------------------------------------------------------------------
.PHONY: clean cleanest
clean:
	/bin/rm -f $(OBJDIR)/*.o

cleanest:
	/bin/rm -f $(OBJDIR)/*.o *.log *.cuts *.lp $(EXE)
#---------------------------------------------------------------------------------------------------
